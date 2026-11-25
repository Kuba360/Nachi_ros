#include "nachi_mz04/nachi_comm.hpp"


#include <OpenNR-IF.h>



#define MEC_NO (1)
#define AXIS_MAX (6)


namespace nachi_mz04{

    bool OpenNRIF::connect(const std::string ip){
        //std::vector<char> nachi_ip_char(ip.begin(),ip.end());
        //nachi_ip_char.push_back('\0');

        char nachi_ip_char[14];
        strncpy(nachi_ip_char,ip.c_str(),sizeof(nachi_ip_char));


        RCLCPP_INFO(logger_,"NACHI ip: %s",nachi_ip_char);
        NACHI_COMMIF_INFO Info;
        ZeroMemory(&Info, sizeof(Info));
        Info.pcAddrs = nachi_ip_char;
        Info.lKind = NR_DATA_XML;
        nXmlOpenId = NR_Open(&Info);
        
        if (0 < nXmlOpenId) {
            RCLCPP_INFO(logger_,"Connected with robot");
            connected_=true;

            BOOL bVal = FALSE;
            if (0 == NR_AcsFixedIOPlayBack(nXmlOpenId, &bVal)) {
                RCLCPP_INFO(logger_,"Playback mode ON");
                
            } else {
                RCLCPP_INFO(logger_,"Playback mode OFF");
            }
        }else {
            RCLCPP_INFO(logger_,"Failed to connect with %d",nXmlOpenId);
            return false;
        }
        return true;
   }

   void OpenNRIF::disconnect(){

        Sleep(1000);
        NR_Close(nXmlOpenId);
        Sleep(1000);

        RCLCPP_INFO(logger_, "Successfully deactivated!");
   }





    bool OpenNRIF::read_axis(float *fValue){
        nErr = NR_AcsAxisTheta(nXmlOpenId, fValue, MEC_NO, NUM_OF_AXES,2,0.5f);
       

        if (0 == nErr) {
            // std::stringstream ss;
            // ss<<"Position ";
            // for (int nAxis = 0; 6 > nAxis; nAxis++){
            //     ss<<"J"<<(nAxis+1)<<":"<<std::fixed<<std::setprecision(1)<<fValue[nAxis];    
            //     if(nAxis<5){
            //         ss<<", ";
            //     }
            // }
            // std::string message=ss.str();
            // //std::cout<<message<<std::endl;
            // RCLCPP_INFO(logger_,"%s",message.c_str());
        }else {
            RCLCPP_INFO(logger_,"\rNR_AcsAxisTheta error : %d", nErr);
            return false;
        }
        return true;   
    }

    bool OpenNRIF::activate_motors(){
        nErr = NR_AcsFixedIOConfirmMotorsOn(nXmlOpenId, &bVal);
        if (0 == nErr) {
            RCLCPP_INFO(logger_,"MOTORS : ON");
        }else{
            RCLCPP_INFO(logger_,"MOTORS : OFF");
            return false;
        }
        return true;
    }

    bool OpenNRIF::set_motors_speed(int SetManSpeed, int SetChkSpeed){

        if(SetManSpeed>5 || SetChkSpeed>5 || SetChkSpeed<0 || SetManSpeed<0){
            RCLCPP_INFO(logger_,"Set speed is to high.");
            return false;
        }

        int nManSpeed = 0;
        int nChkSpeed = 0;
        
        nErr = NR_AcsManualSpeed(nXmlOpenId, &nManSpeed, FALSE);
        nErr2 = NR_AcsCheckSpeed(nXmlOpenId, &nChkSpeed, FALSE);
        if ((0 == nErr)||((0 == nErr2)))
        {
            RCLCPP_INFO(logger_,"Current Manual Speed   : %d       Current CheckSpeed   : % d ", nManSpeed, nChkSpeed);
        }

        nErr = NR_AcsManualSpeed(nXmlOpenId, &SetManSpeed, TRUE);
        nErr2 = NR_AcsCheckSpeed(nXmlOpenId, &SetChkSpeed, TRUE);
        nManSpeed = 0;
        nChkSpeed = 0;
        nErr = NR_AcsManualSpeed(nXmlOpenId, &nManSpeed, FALSE);
        nErr2 = NR_AcsCheckSpeed(nXmlOpenId, &nChkSpeed, FALSE);
        if ((0== nErr) || ((0 == nErr2)))
        {
            RCLCPP_INFO(logger_,"New Manual Speed set to: %d      New Check Speed set to: % d ", nManSpeed, nChkSpeed);
        }

        RCLCPP_INFO(logger_, "Successfully configured speed of motors!");
        return true;
                
    }

    bool OpenNRIF::move1(float *fValue){
        nErr = NR_CtrlMoveJ(nXmlOpenId, fValue, AXIS_MAX, 1);

        if (nErr!=0)
        {
            RCLCPP_INFO(logger_,"NR_CtrlMoveJA error : %d", nErr);
            return false;
        }

        return true;
    }





    bool OpenNRIF::configure_recordings(){
        int x=0;
//SPEED
        nErr=NR_AcsRecordSpeed(nXmlOpenId,&x,false);
        if(nErr!=0){
            RCLCPP_INFO(logger_,"NR_AcsRecordSpeed error : %d", nErr);
            return false;
        }
        RCLCPP_INFO(logger_, "Initial recording speed: %d",x);

        x=80;
        nErr=NR_AcsRecordSpeed(nXmlOpenId,&x,true);
        if(nErr!=0){
            RCLCPP_INFO(logger_,"NR_AcsRecordSpeed error : %d", nErr);
            return false;
        }
        RCLCPP_INFO(logger_, "Set recording speed: %d",x);


//ACCURACY
        nErr = NR_AcsRecordAccuracyNo(nXmlOpenId, &x, false);
        if(nErr!=0){
            RCLCPP_INFO(logger_,"NR_AcsRecordAccuracyNo error : %d", nErr);
            return false;
        }
        RCLCPP_INFO(logger_, "Initial recording accuracy: %d",x);

        x=8;
        nErr = NR_AcsRecordAccuracyNo(nXmlOpenId, &x, true);
        if(nErr!=0){
            RCLCPP_INFO(logger_,"NR_AcsRecordAccuracyNo error : %d", nErr);
            return false;
        }
        RCLCPP_INFO(logger_, "Set recording accuracy: %d",x);


//SMOOTHNESS
        nErr = NR_AcsRecordSmoothNo(nXmlOpenId, &x, false);
        if(nErr!=0){
            RCLCPP_INFO(logger_,"NR_AcsRecordSmoothNo error : %d", nErr);
            return false;
        }
        RCLCPP_INFO(logger_, "Initial recording smoothness: %d",x);

        x=3;
        nErr = NR_AcsRecordSmoothNo(nXmlOpenId, &x, true);
        if(nErr!=0){
            RCLCPP_INFO(logger_,"NR_AcsRecordSmoothNo error : %d", nErr);
            return false;
        }
        RCLCPP_INFO(logger_, "Set recording smoothness: %d",x);


//ACCELERATION
        nErr = NR_AcsAccelerationNo(nXmlOpenId, &x, false);
        if(nErr!=0){
            RCLCPP_INFO(logger_,"NR_AcsAccelerationNo error : %d", nErr);
            return false;
        }
        RCLCPP_INFO(logger_, "Initial recording acceleration: %d",x);

        x=3;
        nErr = NR_AcsAccelerationNo(nXmlOpenId, &x, true);
        if(nErr!=0){
            RCLCPP_INFO(logger_,"NR_AcsAccelerationNo error : %d", nErr);
            return false;
        }
        RCLCPP_INFO(logger_, "Set recording acceleration: %d",x);


        return true;      
    }


        

}
