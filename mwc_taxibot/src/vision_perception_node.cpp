#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <std_msgs/Bool.h>
#include <std_msgs/String.h>

#include <cstddef>
#include <cstdint>
#include <string>

class VisionPerceptionNode
{
public:
    VisionPerceptionNode()
        : nh_(),
          pnh_("~"),
          visual_obstacle_(false),
          enable_color_zone_detection_(true),
          publish_only_after_image_(true)
    {
        pnh_.param<std::string>("image_topic", image_topic_, "/camera/image_raw");
        pnh_.param<std::string>("simulated_zone_label", simulated_zone_label_, "");
        pnh_.param("visual_obstacle", visual_obstacle_, false);
        pnh_.param("enable_color_zone_detection", enable_color_zone_detection_, true);
        pnh_.param("publish_only_after_image", publish_only_after_image_, true);
        pnh_.param("publish_rate", publish_rate_, 2.0);
        if (publish_rate_ <= 0.0)
        {
            publish_rate_ = 2.0;
        }

        image_sub_ = nh_.subscribe(image_topic_, 1, &VisionPerceptionNode::imageCallback, this);
        obstacle_pub_ = nh_.advertise<std_msgs::Bool>("/visual_obstacle", 1, true);
        zone_pub_ = nh_.advertise<std_msgs::String>("/zone_detection", 1, true);
        timer_ = nh_.createTimer(ros::Duration(1.0 / publish_rate_), &VisionPerceptionNode::timerCallback, this);

        ROS_INFO_STREAM("vision_perception_node escuchando " << image_topic_);
    }

private:
    void imageCallback(const sensor_msgs::Image::ConstPtr& image)
    {
        received_image_ = true;
        if (enable_color_zone_detection_ && simulated_zone_label_.empty())
        {
            detected_zone_label_ = detectColorMarker(*image);
        }
        publishPerception();
    }

    void timerCallback(const ros::TimerEvent&)
    {
        pnh_.getParam("visual_obstacle", visual_obstacle_);
        pnh_.getParam("simulated_zone_label", simulated_zone_label_);

        if (!publish_only_after_image_ || received_image_)
        {
            publishPerception();
        }
    }

    void publishPerception()
    {
        std_msgs::Bool obstacle_msg;
        obstacle_msg.data = visual_obstacle_;
        obstacle_pub_.publish(obstacle_msg);

        std_msgs::String zone_msg;
        zone_msg.data = simulated_zone_label_.empty() ? detected_zone_label_ : simulated_zone_label_;
        zone_pub_.publish(zone_msg);
    }

    static std::string detectColorMarker(const sensor_msgs::Image& image)
    {
        if (image.data.empty() || image.width == 0 || image.height == 0)
        {
            return "";
        }

        const bool rgb = image.encoding == "rgb8";
        const bool bgr = image.encoding == "bgr8";
        if (!rgb && !bgr)
        {
            return "";
        }

        std::uint64_t red_score = 0;
        std::uint64_t blue_score = 0;
        std::uint64_t green_score = 0;
        std::uint64_t yellow_score = 0;
        std::uint64_t purple_score = 0;
        const std::uint32_t step_y = image.height > 80 ? image.height / 80 : 1;
        const std::uint32_t step_x = image.width > 80 ? image.width / 80 : 1;

        for (std::uint32_t y = 0; y < image.height; y += step_y)
        {
            for (std::uint32_t x = 0; x < image.width; x += step_x)
            {
                const std::size_t index = y * image.step + x * 3;
                if (index + 2 >= image.data.size())
                {
                    continue;
                }

                const int c0 = image.data[index];
                const int c1 = image.data[index + 1];
                const int c2 = image.data[index + 2];
                const int r = rgb ? c0 : c2;
                const int g = c1;
                const int b = rgb ? c2 : c0;

                if (r > 150 && g < 110 && b < 110)
                {
                    red_score += 1;
                }
                if (b > 150 && r < 120 && g < 140)
                {
                    blue_score += 1;
                }
                if (g > 140 && r < 130 && b < 130)
                {
                    green_score += 1;
                }
                if (r > 150 && g > 130 && b < 100)
                {
                    yellow_score += 1;
                }
                if (r > 120 && b > 120 && g < 110)
                {
                    purple_score += 1;
                }
            }
        }

        std::string label;
        std::uint64_t best_score = 30;
        if (red_score > best_score)
        {
            label = "ENTRADA";
            best_score = red_score;
        }
        if (blue_score > best_score)
        {
            label = "AUDITORIO";
            best_score = blue_score;
        }
        if (green_score > best_score)
        {
            label = "ROBOTICA";
            best_score = green_score;
        }
        if (yellow_score > best_score)
        {
            label = "CAFE";
            best_score = yellow_score;
        }
        if (purple_score > best_score)
        {
            label = "NETWORKING";
        }
        return label;
    }

    ros::NodeHandle nh_;
    ros::NodeHandle pnh_;
    ros::Subscriber image_sub_;
    ros::Publisher obstacle_pub_;
    ros::Publisher zone_pub_;
    ros::Timer timer_;
    std::string image_topic_;
    std::string simulated_zone_label_;
    std::string detected_zone_label_;
    bool visual_obstacle_;
    bool enable_color_zone_detection_;
    bool publish_only_after_image_;
    bool received_image_ = false;
    double publish_rate_;
};

int main(int argc, char** argv)
{
    ros::init(argc, argv, "vision_perception_node");
    VisionPerceptionNode node;
    ros::spin();
    return 0;
}
