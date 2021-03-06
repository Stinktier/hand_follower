#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Point.h>

class hand_controller{
	public:
	ros::NodeHandle n;
    ros::Publisher Twist_publisher;
    geometry_msgs::Twist msg;
	ros::Subscriber Hand_subscriber;

	hand_controller(double x, double y){
		n = ros::NodeHandle("~");
		Hand_subscriber = n.subscribe("/hand_tracker/direction", 1, &hand_controller::directioncallbacker,this);
		Twist_publisher= n.advertise<geometry_msgs::Twist>("/motor_controller/twist", 100);
		error[0]=0;
		error[1]=0;
		aimed[0]=x;
		aimed[1]=y;
		current[0]=x;
		current[0]=y;
		speed=0;
		angel=0;
		speedparameter = 1;
		angularparameter= 5;
		if(n.hasParam("Xtarget")){
			n.getParam("Xtarget",aimed[0]);
			ROS_INFO("Xtarget was set");
		}
		if(n.hasParam("Ytarget")){
			n.getParam("Ytarget",aimed[1]);
			ROS_INFO("Ytarget was set");
		}
		if(n.hasParam("speedP")){
			n.getParam("speedP",speedparameter);
			ROS_INFO("Speedparameter was set");
		}
		if(n.hasParam("angularP")){
			n.getParam("angularP",angularparameter);
			ROS_INFO("angularParameter was set");
		}

	}
    void call(){
		error[0]= aimed[0] - current[0];
		error[1]= aimed[1] - current[1];
		speed=speed+ speedparameter*error[0];
		angel=angel+ angularparameter*error[1];
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
		if(abs(current[0])<0.000001 && abs(current[1])<0.000001){
				current[0]=aimed[0];
				current[0]=aimed[0];
		}
		
	}
	private:
	double speedparameter;
	double angularparameter;
	double current [2];
	double error [2]; 
	double aimed [2];
	double speed, angel;
};
int main(int argc, char **argv)
{
    ros::init(argc, argv, "hand_controller");
	double xvalue=0.6;
	double yvalue=0;
    hand_controller hand_controller_node(xvalue,yvalue);
    ros::Rate loop_rate(10.0);

    while(hand_controller_node.n.ok())
    {
    	
    	hand_controller_node.call();
        ros::spinOnce();
        loop_rate.sleep();
    }

    return 0;
}
