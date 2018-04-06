// Copyright 2017 ADLINK Technology, Inc.
// Developer: Alan Chen (alan.chen@adlinktech.com)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <stdio.h>
#include "neuronIIcNode.hpp"


/* * * * * * * * * * 
 * Private Methods *
 * * * * * * * * * */
 void NeuronIIcNode::topic_callback(const std_msgs::msg::String::SharedPtr msg)
{
    // Print the received message
    printf("------------------------------------------------------------------\n");
    printf("Topic \"%s\" called: \"%s\".\n", TOPIC_CMD, msg->data.c_str());

    // Check the availability of the SEMA library 
    if (NeuronIIc::IsAvailable() == false || gpio_ == NULL)
    {
        printf("[ERROR] NeuronIIcNode - SEMA Lib not found.\n");
        return;
    }
	/*
    // Toggle GPIO Level
    uint32_t level = 0;
    gpio_->SetDir(EAPI_GPIO_OUTPUT);
    gpio_->ReadLevel(level);
    //level = (level == EAPI_GPIO_LOW);
    gpio_->SetLevel(!!!level);

    // Send it out
    std::string stmp;
    stmp += "The GPIO Pin";
    stmp += std::to_string(GPIO_TOGGLE_PIN);
    stmp += " is set to ";
    stmp += level? "HIGH" : "LOW";
	*/
	//msg->data = stmp;
	//printf("<<<= send to ------- Topic <\"%s\">: \"%s\".\n", TOPIC_DATA, msg->data.c_str());
	publisher_->publish(msg);
	
	uint8_t data[14];
    gpio_->WakeUp6050();
	gpio_->ReadI2C(data, 14);
    
    //==== IMU msg is currently unavailable under ROS2 ====//
    /* method 1
    sensor_msgs::msg::Imu imu;
	imu.linear_acceleration.x = data[0];
	imu.linear_acceleration.y = data[1];
	imu.linear_acceleration.z = data[2];
	imu.angular_velocity.x = data[4];
	imu.angular_velocity.y = data[5];
	imu.angular_velocity.z = data[6];*/
    
    /* method 2
    auto imu_msg = std::make_shared<sensor_msgs::msg::Imu>();
    imu_msg->linear_acceleration.y = data[1];
	imu_msg->linear_acceleration.z = data[2];
	tmp = data[3];
	imu_msg->angular_velocity.x = data[4];
	imu_msg->angular_velocity.y = data[5];
	imu_msg->angular_velocity.z = data[6];*/
	
	int16_t acX = data[0]<<8|data[1];
	int16_t acY = data[2]<<8|data[3];
    int16_t acZ = data[4]<<8|data[5];
    int16_t tmp = data[6]<<8|data[7];
    int16_t gyX = data[8]<<8|data[9];
    int16_t gyY = data[10]<<8|data[11];
    int16_t gyZ = data[12]<<8|data[13];
    	
    printf("MPU6050 acceleration x:%d, y:%d, z:%d\n", acX, acY, acZ);
	printf("MPU6050 rotation x:%d, y:%d, z:%d\n", gyX, gyY, gyZ);
	printf("at temperature: %f\n", tmp/340.00+36.53);
    //publisher_->publish(imu_msg);
    return;
}


/* * * * * * * * * * 
 * Public Methods  *
 * * * * * * * * * */
NeuronIIcNode::NeuronIIcNode() : Node("neuron_gpio")
{
    //==== IMU msg is currently unavailable under ROS2 ====//
    /*publisher_ = this->create_publisher<sensor_msgs::msg::Imu>(
            TOPIC_DATA, rmw_qos_profile_sensor_data);*/
    
    publisher_ = this->create_publisher<std_msgs::msg::String>(
            TOPIC_DATA, rmw_qos_profile_sensor_data);

    subscription_ = this->create_subscription<std_msgs::msg::String>(
            TOPIC_CMD, std::bind(&NeuronIIcNode::topic_callback, this, _1),
            rmw_qos_profile_sensor_data);
        
    NeuronIIc::InitLib();
    
    gpio_ = NeuronIIc::IsAvailable()? std::make_shared<NeuronIIc>(GPIO_TOGGLE_PIN) : NULL;
}

NeuronIIcNode::~NeuronIIcNode()
{
    NeuronIIc::UnInitLib();
}
