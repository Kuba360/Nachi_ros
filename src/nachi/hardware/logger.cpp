#include <sstream>
#include <iomanip>
#include <numbers>
#include <vector>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>

class LoggerNode : public rclcpp::Node
{
public:
    LoggerNode() : Node("nachi_logger")
    {
        subscription_ =
            this->create_subscription<sensor_msgs::msg::JointState>(
                "/joint_states",
                10,
                std::bind(
                    &LoggerNode::topic_callback,
                    this,
                    std::placeholders::_1
                )
            );
        RCLCPP_INFO(this->get_logger(),"Logger started");
        last_log_time_ = this->now();
    }

private:
    rclcpp::Time last_log_time_;
    std::vector<double> last_positions_;
    bool first_message_ = true;
    rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr subscription_;
    int stop=0;
    

    void topic_callback(const sensor_msgs::msg::JointState::SharedPtr msg){

        if ((this->now() - last_log_time_).seconds() < 0.1) return;
        last_log_time_ = this->now();

        const auto & positions = msg->position;

        bool changed = false;
        if (first_message_ || positions != last_positions_) changed=true;
        if (!changed){
            stop++;
            if(stop==10) RCLCPP_INFO(this->get_logger(),"STOP");   
            return;
        }
        stop=0;
        first_message_ = false;
        last_positions_ = positions;


        std::stringstream ss;
        ss<<"Position - ";
        for (int i=0;i<6;i++){
            ss<<"J"<<(i+1)<<":"<<std::fixed<<std::setprecision(1)<<(positions[i]*(180.0/std::numbers::pi));    
            if(i<5){
                ss<<", ";
            }
        }
        std::string message=ss.str();
        RCLCPP_INFO(this->get_logger(),"%s",message.c_str());

    }


};

int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);

    rclcpp::spin(std::make_shared<LoggerNode>());

    rclcpp::shutdown();

    return 0;
}
