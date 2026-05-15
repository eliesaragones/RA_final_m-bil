#include "mwc_taxibot/taxi_core.hpp"

#include <actionlib/client/simple_action_client.h>
#include <geometry_msgs/PoseStamped.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <mwc_taxibot/RequestTaxi.h>
#include <mwc_taxibot/TaxiStatus.h>
#include <ros/ros.h>
#include <std_msgs/Bool.h>
#include <std_msgs/String.h>
#include <tf2/LinearMath/Quaternion.h>
#include <XmlRpcValue.h>

#include <boost/bind.hpp>

#include <string>

class TaxiManagerNode
{
public:
    TaxiManagerNode()
        : private_nh_("~"),
          move_base_client_(loadMoveBaseActionName(), true),
          obstacle_active_(false),
          obstacle_retries_(0)
    {
        loadParameters();
        loadZones();

        request_server_ = nh_.advertiseService("/request_taxi", &TaxiManagerNode::handleTaxiRequest, this);
        status_pub_ = nh_.advertise<mwc_taxibot::TaxiStatus>("/taxi_status", 10, true);
        visual_obstacle_sub_ = nh_.subscribe("/visual_obstacle", 10, &TaxiManagerNode::handleVisualObstacle, this);
        zone_detection_sub_ = nh_.subscribe("/zone_detection", 10, &TaxiManagerNode::handleZoneDetection, this);

        const ros::Duration timer_period(1.0 / status_publish_rate_);
        status_timer_ = nh_.createTimer(timer_period, &TaxiManagerNode::handleTimer, this);

        current_zone_ = base_zone_;
        ROS_INFO("MWC TaxiBot listo. Esperando servidor de navegación '%s'.", move_base_action_name_.c_str());
        move_base_client_.waitForServer(ros::Duration(2.0));
        publishStatus();
    }

private:
    using MoveBaseClient = actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction>;

    static std::string loadMoveBaseActionName()
    {
        ros::NodeHandle private_nh("~");
        std::string action_name;
        private_nh.param<std::string>("move_base_action_name", action_name, "move_base");
        return action_name;
    }

    void loadParameters()
    {
        private_nh_.param<std::string>("base_zone", base_zone_, "base");
        private_nh_.param<double>("pickup_wait_time", pickup_wait_time_, 5.0);
        private_nh_.param<double>("zone_confirmation_timeout", zone_confirmation_timeout_, 4.0);
        private_nh_.param<double>("obstacle_wait_time", obstacle_wait_time_, 3.0);
        private_nh_.param<int>("max_obstacle_retries", max_obstacle_retries_, 3);
        private_nh_.param<bool>("return_to_base_after_service", return_to_base_after_service_, true);
        private_nh_.param<bool>("auto_confirm_zone_without_vision", auto_confirm_zone_without_vision_, true);
        private_nh_.param<double>("status_publish_rate", status_publish_rate_, 2.0);
        private_nh_.param<std::string>("move_base_action_name", move_base_action_name_, "move_base");

        if (status_publish_rate_ <= 0.0)
        {
            status_publish_rate_ = 2.0;
        }

        taxi_core_.setBaseZone(base_zone_);
        taxi_core_.setReturnToBaseAfterService(return_to_base_after_service_);
    }

    void loadZones()
    {
        XmlRpc::XmlRpcValue zones;
        if (!private_nh_.getParam("zones", zones) || zones.getType() != XmlRpc::XmlRpcValue::TypeStruct)
        {
            ROS_WARN("No se han cargado zonas en '~zones'. Se usará una zona base mínima.");
            taxi_core_.addZone({"base", 0.0, 0.0, 0.0, "base"});
            return;
        }

        for (auto it = zones.begin(); it != zones.end(); ++it)
        {
            const std::string name = it->first;
            XmlRpc::XmlRpcValue zone_data = it->second;
            if (zone_data.getType() != XmlRpc::XmlRpcValue::TypeStruct)
            {
                ROS_WARN("Zona '%s' ignorada: formato YAML inválido.", name.c_str());
                continue;
            }

            mwc_taxibot::Zone zone;
            zone.name = name;
            zone.x = readDouble(zone_data, "x", 0.0);
            zone.y = readDouble(zone_data, "y", 0.0);
            zone.yaw = readDouble(zone_data, "yaw", 0.0);
            zone.visual_label = readString(zone_data, "visual_label", name);
            taxi_core_.addZone(zone);
        }

        if (!taxi_core_.hasZone(base_zone_))
        {
            ROS_WARN("La zona base '%s' no existe. Se añade en (0, 0, 0).", base_zone_.c_str());
            taxi_core_.addZone({base_zone_, 0.0, 0.0, 0.0, base_zone_});
        }
    }

    static double readDouble(XmlRpc::XmlRpcValue& value, const std::string& key, const double fallback)
    {
        if (!value.hasMember(key))
        {
            return fallback;
        }
        XmlRpc::XmlRpcValue field = value[key];
        if (field.getType() == XmlRpc::XmlRpcValue::TypeInt)
        {
            return static_cast<int>(field);
        }
        if (field.getType() == XmlRpc::XmlRpcValue::TypeDouble)
        {
            return static_cast<double>(field);
        }
        return fallback;
    }

    static std::string readString(XmlRpc::XmlRpcValue& value, const std::string& key, const std::string& fallback)
    {
        if (!value.hasMember(key) || value[key].getType() != XmlRpc::XmlRpcValue::TypeString)
        {
            return fallback;
        }
        return static_cast<std::string>(value[key]);
    }

    bool handleTaxiRequest(mwc_taxibot::RequestTaxi::Request& request,
                           mwc_taxibot::RequestTaxi::Response& response)
    {
        const auto result = taxi_core_.requestTaxi(request.pickup_zone, request.destination_zone);
        response.accepted = result.accepted;
        response.message = result.message;

        if (result.accepted)
        {
            obstacle_retries_ = 0;
            obstacle_active_ = false;
            sendCurrentGoal();
        }

        publishStatus();
        return true;
    }

    void sendCurrentGoal()
    {
        if (!taxi_core_.hasActiveGoal())
        {
            return;
        }

        const auto& zone = taxi_core_.currentGoal();
        move_base_msgs::MoveBaseGoal goal;
        goal.target_pose.header.frame_id = "map";
        goal.target_pose.header.stamp = ros::Time::now();
        goal.target_pose.pose.position.x = zone.x;
        goal.target_pose.pose.position.y = zone.y;
        goal.target_pose.pose.position.z = 0.0;

        tf2::Quaternion quaternion;
        quaternion.setRPY(0.0, 0.0, zone.yaw);
        goal.target_pose.pose.orientation.x = quaternion.x();
        goal.target_pose.pose.orientation.y = quaternion.y();
        goal.target_pose.pose.orientation.z = quaternion.z();
        goal.target_pose.pose.orientation.w = quaternion.w();

        move_base_client_.sendGoal(
            goal,
            boost::bind(&TaxiManagerNode::handleNavigationDone, this, _1, _2),
            MoveBaseClient::SimpleActiveCallback(),
            MoveBaseClient::SimpleFeedbackCallback());

        ROS_INFO("Nuevo objetivo enviado: %s (%.2f, %.2f, %.2f)",
                 zone.name.c_str(),
                 zone.x,
                 zone.y,
                 zone.yaw);
    }

    void handleNavigationDone(const actionlib::SimpleClientGoalState& state,
                              const move_base_msgs::MoveBaseResultConstPtr&)
    {
        if (taxi_core_.state() == mwc_taxibot::TaxiState::OBSTACLE_DETECTED)
        {
            return;
        }

        if (state == actionlib::SimpleClientGoalState::SUCCEEDED)
        {
            const auto previous_state = taxi_core_.state();
            const std::string previous_goal = taxi_core_.hasActiveGoal() ? taxi_core_.currentGoal().name : "";
            taxi_core_.markNavigationSucceeded();

            if (previous_state == mwc_taxibot::TaxiState::GOING_TO_PICKUP)
            {
                current_zone_ = previous_goal;
                passenger_wait_started_ = ros::Time::now();
            }
            else if (previous_state == mwc_taxibot::TaxiState::GOING_TO_DESTINATION)
            {
                zone_confirmation_started_ = ros::Time::now();
            }
            else if (previous_state == mwc_taxibot::TaxiState::RETURNING_TO_BASE)
            {
                current_zone_ = base_zone_;
            }
        }
        else
        {
            taxi_core_.markNavigationFailed("move_base no pudo alcanzar el objetivo: " + state.toString());
        }

        publishStatus();
    }

    void handleVisualObstacle(const std_msgs::Bool::ConstPtr& message)
    {
        obstacle_active_ = message->data;
        if (message->data)
        {
            if (taxi_core_.state() != mwc_taxibot::TaxiState::OBSTACLE_DETECTED)
            {
                obstacle_retries_ += 1;
                obstacle_started_ = ros::Time::now();
            }
            taxi_core_.markVisualObstacle(true);
        }
        else if (taxi_core_.state() == mwc_taxibot::TaxiState::OBSTACLE_DETECTED)
        {
            taxi_core_.markVisualObstacle(false);
            sendCurrentGoal();
        }
        publishStatus();
    }

    void handleZoneDetection(const std_msgs::String::ConstPtr& message)
    {
        latest_zone_label_ = message->data;
    }

    void handleTimer(const ros::TimerEvent&)
    {
        const auto now = ros::Time::now();

        if (taxi_core_.state() == mwc_taxibot::TaxiState::WAITING_PASSENGER &&
            (now - passenger_wait_started_).toSec() >= pickup_wait_time_)
        {
            taxi_core_.markPassengerWaitComplete();
            sendCurrentGoal();
        }

        if (taxi_core_.state() == mwc_taxibot::TaxiState::CONFIRMING_ZONE)
        {
            const bool confirmed = auto_confirm_zone_without_vision_ || isDestinationVisuallyConfirmed();
            if (confirmed)
            {
                current_zone_ = taxi_core_.destinationZone();
                taxi_core_.markZoneConfirmed(true);
                sendCurrentGoal();
            }
            else if ((now - zone_confirmation_started_).toSec() >= zone_confirmation_timeout_)
            {
                taxi_core_.markZoneConfirmed(false);
            }
        }

        if (taxi_core_.state() == mwc_taxibot::TaxiState::OBSTACLE_DETECTED &&
            obstacle_active_ &&
            (now - obstacle_started_).toSec() >= obstacle_wait_time_ &&
            obstacle_retries_ >= max_obstacle_retries_)
        {
            move_base_client_.cancelGoal();
            taxi_core_.markNavigationFailed("Obstáculo visual persistente. Servicio cancelado.");
        }

        publishStatus();
    }

    bool isDestinationVisuallyConfirmed() const
    {
        if (!taxi_core_.hasZone(taxi_core_.destinationZone()))
        {
            return false;
        }
        const auto& destination = taxi_core_.zone(taxi_core_.destinationZone());
        return !destination.visual_label.empty() && latest_zone_label_ == destination.visual_label;
    }

    void publishStatus()
    {
        mwc_taxibot::TaxiStatus status;
        status.state = mwc_taxibot::toString(taxi_core_.state());
        status.current_zone = current_zone_;
        status.pickup_zone = taxi_core_.pickupZone();
        status.destination_zone = taxi_core_.destinationZone();
        status.message = taxi_core_.statusMessage();
        status.busy = taxi_core_.busy();
        status_pub_.publish(status);
    }

    ros::NodeHandle nh_;
    ros::NodeHandle private_nh_;
    MoveBaseClient move_base_client_;
    mwc_taxibot::TaxiCore taxi_core_;

    ros::ServiceServer request_server_;
    ros::Publisher status_pub_;
    ros::Subscriber visual_obstacle_sub_;
    ros::Subscriber zone_detection_sub_;
    ros::Timer status_timer_;

    std::string base_zone_;
    std::string current_zone_;
    std::string latest_zone_label_;
    std::string move_base_action_name_;

    double pickup_wait_time_;
    double zone_confirmation_timeout_;
    double obstacle_wait_time_;
    double status_publish_rate_;
    int max_obstacle_retries_;
    bool return_to_base_after_service_;
    bool auto_confirm_zone_without_vision_;
    bool obstacle_active_;
    int obstacle_retries_;

    ros::Time passenger_wait_started_;
    ros::Time zone_confirmation_started_;
    ros::Time obstacle_started_;
};

int main(int argc, char** argv)
{
    ros::init(argc, argv, "taxi_manager_node");
    TaxiManagerNode node;
    ros::spin();
    return 0;
}
