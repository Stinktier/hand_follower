#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Point.h>

class hand_controller{
	ros::NodeHandle n;
    ros::Publisher Twist_publisher;
    geometry_msgs::Twist msg;
	ros::Subscriber Hand_subscriber;
	
	hand_controller(){
		n = ros::NodeHandle("~");
	}
	// Z ist the Distance to the object, x is distance to the middle. 
	void init(double x, double z){        
		Hand_subscriber = n.subscribe("/hand_tracker/direction", 1, &hand_controller::directioncallbacker,this);
		Twist_publisher= n.advertise<geometry_msgs::Twist>("/motor_control/Twist", 100);
		error[0]=0;
		error[1]=0;
		aimed[0]=x;
		aimed[1]=z;
		current[0]=x;
		current[0]=z;
		ID=0;
		speed=0;
		angel=0;
    }
    void call(){
		error[0]= aimed[0] - current[0];
		error[1]= aimed[1] - current[1];
		speed=speed+speedparameter*error[1];
		angel=angel+ angularparameter*error[0];
		msg.linear.x=speed;
    	msg.linear.y=0;
    	msg.linear.z=0;
    	msg.angular.x=0;
    	msg.angular.y=0;
    	msg.angular.z=angel;
    	Twist_publisher.publish(msg);
	}
	void directioncallbacker(geometry_msgs::Point msgP){
		current[0]=msgP.x;
		current[1]=msgP.z;
		if(current[0]==0 && current[1]==0){
				current[0]=aimed[0];
				current[0]=aimed[0]
		}
		
	}
	private
	static const float speedparameter = 0.05;
	static const float angularparameter= 0.05;
	double current [2];
	double error [2]; 
	double aimed [2];
	double speed, angel;
}
