#include <ros/ros.h>
#include <limits>
#include <sensor_msgs/PointCloud2.h>
#include <geometry_msgs/Point.h>
#include <visualization_msgs/Marker.h>
#include <tf/transform_broadcaster.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/registration/icp.h>
#include <pcl/filters/crop_box.h>
#include <pcl/common/centroid.h>

#define BOXSIZE 0.5
#define DEPTH_OFFSET 0.3
#define HEIGHT_OFFSET 0.3

typedef pcl::PCLPointCloud2 Cloud2;
typedef pcl::PointXYZRGB Point;
typedef pcl::PointCloud<Point> Cloud;

static ros::Publisher pcl_pub;
static ros::Publisher dir_pub;
static ros::Publisher vis_pub;

void imageCb(const sensor_msgs::PointCloud2ConstPtr& pcl_msg) {
    static bool tracking = false;

    Cloud::Ptr input(new pcl::PointCloud<pcl::PointXYZRGB>);
    Cloud::Ptr output(new pcl::PointCloud<pcl::PointXYZRGB>);
    Cloud2 tmp_pcl;
    pcl_conversions::toPCL(*pcl_msg, tmp_pcl);
    pcl::fromPCLPointCloud2(tmp_pcl, *input);

    //Order of params: width, height, depth
    Eigen::Vector4f minVal(-BOXSIZE, -BOXSIZE + HEIGHT_OFFSET, DEPTH_OFFSET, 1);
    Eigen::Vector4f maxVal(BOXSIZE, BOXSIZE + HEIGHT_OFFSET, 2*BOXSIZE + DEPTH_OFFSET, 1);

    pcl::CropBox<Point> cb;
    cb.setMin(minVal);
    cb.setMax(maxVal);
    cb.setInputCloud(input);
    cb.filter(*output);

    //Do we have enough points to track
    tracking = output->size() > 5000;

    Eigen::Vector4f massCenter;
    massCenter[0] = 0;
    massCenter[1] = 0;
    massCenter[2] = 0;
    if(tracking) {
        pcl::ConstCloudIterator<Point> cloudIterator(*output);
        pcl::compute3DCentroid(cloudIterator, massCenter);
    }

    sensor_msgs::PointCloud2 pcl_msg_out;
    pcl::toROSMsg(*output, pcl_msg_out);
    pcl_pub.publish(pcl_msg_out);

    geometry_msgs::Point dir_msg_out;
    dir_msg_out.x = massCenter[2];
    dir_msg_out.y = -massCenter[0];
    dir_msg_out.z = -massCenter[1];
    dir_pub.publish(dir_msg_out);

    visualization_msgs::Marker vis_msg_out;
    vis_msg_out.header.frame_id = "/camera_depth_frame";
    vis_msg_out.header.stamp = ros::Time();
    vis_msg_out.ns = "dir_marker";
    vis_msg_out.id = 0;
    vis_msg_out.action = visualization_msgs::Marker::ADD;
    vis_msg_out.type = visualization_msgs::Marker::ARROW;
    vis_msg_out.points.push_back(geometry_msgs::Point());
    vis_msg_out.lifetime = ros::Duration(3, 0);
    std_msgs::ColorRGBA c; c.r = 1; c.a = 0.5;
    vis_msg_out.color = c;
    vis_msg_out.points.push_back(dir_msg_out);
    vis_msg_out.scale.x =  0.04;
    vis_msg_out.scale.y =  0.08;
    vis_pub.publish(vis_msg_out);

    //    visualization_msgs::Marker vis_msg_out2;
    //    vis_msg_out2.header.frame_id = "/camera_depth_frame";
    //    vis_msg_out2.header.stamp = ros::Time();
    //    vis_msg_out2.ns = "box_marker";
    //    vis_msg_out2.id = 0;
    //    vis_msg_out2.action = visualization_msgs::Marker::ADD;
    //    vis_msg_out2.type = visualization_msgs::Marker::LINE_STRIP;
    //    geometry_msgs::Point p;
    //    p.x = minVal[2]; p.y = -minVal[1]; p.z = -minVal[0];
    //    vis_msg_out2.points.push_back(p);
    //    p.x = minVal[2]; p.y = -minVal[1]; p.z = -maxVal[0];
    //    vis_msg_out2.points.push_back(p);
    //    vis_msg_out2.points.push_back(p);
    //    p.x = minVal[2]; p.y = -maxVal[1]; p.z = -maxVal[0];
    //    vis_msg_out2.points.push_back(p);
    //    vis_msg_out2.points.push_back(p);
    //    p.x = minVal[2]; p.y = -maxVal[1]; p.z = -minVal[0];
    //    vis_msg_out2.points.push_back(p);
    //    vis_msg_out2.points.push_back(p);
    //    p.x = maxVal[2]; p.y = -maxVal[1]; p.z = -minVal[0];
    //    vis_msg_out2.points.push_back(p);
    //    vis_msg_out2.points.push_back(p);
    //    p.x = maxVal[2]; p.y = -maxVal[1]; p.z = -maxVal[0];
    //    vis_msg_out2.points.push_back(p);
    //    vis_msg_out2.points.push_back(p);
    //    p.x = maxVal[2]; p.y = -minVal[1]; p.z = -maxVal[0];
    //    vis_msg_out2.points.push_back(p);
    //    vis_msg_out2.points.push_back(p);
    //    p.x = maxVal[2]; p.y = -minVal[1]; p.z = -minVal[0];
    //    vis_msg_out2.points.push_back(p);


    //    vis_msg_out2.lifetime = ros::Duration(3, 0);
    //    vis_msg_out2.color = c;
    //    vis_msg_out2.scale.x =  0.04;
    //    vis_pub.publish(vis_msg_out2);
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "hand_tracker");
    ros::NodeHandle nh;
    ros::Subscriber pcl_sub = nh.subscribe("/camera/depth_registered/points", 1, imageCb);
    pcl_pub = nh.advertise<sensor_msgs::PointCloud2>("/hand_tracker/pcl_filtered", 1);
    dir_pub = nh.advertise<geometry_msgs::Point>("/hand_tracker/direction", 1);
    vis_pub = nh.advertise<visualization_msgs::Marker>("/hand_tracker/vis_marker", 0);
    ros::spin();
    return 0;
}
