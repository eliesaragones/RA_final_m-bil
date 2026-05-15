#include "mwc_taxibot/RequestTaxi.h"

#include <ros/ros.h>

#include <string>

int main(int argc, char** argv)
{
    ros::init(argc, argv, "taxi_request_client");

    if (argc != 3)
    {
        ROS_ERROR_STREAM("Uso: rosrun mwc_taxibot taxi_request_client <pickup_zone> <destination_zone>");
        return 1;
    }

    ros::NodeHandle nh;
    ros::ServiceClient client = nh.serviceClient<mwc_taxibot::RequestTaxi>("/request_taxi");

    ROS_INFO_STREAM("Esperando servicio /request_taxi...");
    client.waitForExistence();

    mwc_taxibot::RequestTaxi srv;
    srv.request.pickup_zone = argv[1];
    srv.request.destination_zone = argv[2];

    if (!client.call(srv))
    {
        ROS_ERROR_STREAM("No se pudo llamar a /request_taxi");
        return 2;
    }

    if (srv.response.accepted)
    {
        ROS_INFO_STREAM(srv.response.message);
        return 0;
    }

    ROS_WARN_STREAM(srv.response.message);
    return 3;
}
