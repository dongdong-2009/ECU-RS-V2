#include "ECUCollect.h"
#include "rtthread.h"
#include "datetime.h"
#include "inverter.h"
#include "variation.h"
#include "debug.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "rtc.h"
#include "serverfile.h"
#include "threadlist.h"
#include <dfs_posix.h>
#include "zigbee.h"
#include "timer.h"
#include "rsdFunction.h"
#include "remote_update.h"
#include "third_inverter.h"
#include "protocol_json.h"
#include "TrinaSolar.h"
#include "ZigBeeChannel.h"
#include "ZigBeeTransmission.h"

extern unsigned char rateOfProgress;
extern ecu_info ecu;
extern inverter_info inverterInfo[MAXINVERTERCOUNT];
extern inverter_third_info thirdInverterInfo[MAX_THIRD_INVERTER_COUNT];
unsigned char ECUCommThreadFlag = EN_ECUHEART_DISABLE;
unsigned short comm_failed_Num = 0;

void inverter_Info(inverter_info *curinverter)
{
    printf("\n");

    printf("ID:%s  ",curinverter->uid);
    printf("LastCollectTime: %s  ",curinverter->LastCollectTime);
    printf("LastCommTime:    %s  ",curinverter->LastCommTime);
    printf("PV1_Energy:%d   ",curinverter->PV1_Energy);
    printf("PV2_Energy:%d   ",curinverter->PV2_Energy);
    printf("Last_PV1_Energy:%d   ",curinverter->Last_PV1_Energy);
    printf("Last_PV2_Energy:%d   ",curinverter->Last_PV2_Energy);
    printf("AveragePower1:%lf   ",curinverter->AveragePower1);
    printf("AveragePower2:%lf  ",curinverter->AveragePower2);
    printf("EnergyPV1:%d  ",curinverter->EnergyPV1);
    printf("EnergyPV2:%d \n",curinverter->EnergyPV2);

}

int init_all(inverter_info *inverter)
{
    rateOfProgress = 0;
    openzigbee();
    init_ecu();
    if(-1==zb_test_communication())
        zigbee_reset();
    //zigbee_reset();
    //zb_test_communication();
    init_inverter(inverter);
    rateOfProgress = 100;
    init_Third_Inverter(thirdInverterInfo);
    init_tmpdb(inverter);
    init_rsdStatus(inverter);
    ResponseECUZigbeeChannel(ecu.channel,ecu.panid,0);
    return 0;
}


void Collect_Client_Record(void)
{
    char *client_Data = NULL;
    int length = 0; //��ǰ��������λ��
    int i = 0;				//��ѵ����
    unsigned systemModel = 0xff;	//ϵͳ����
    inverter_info *curinverter = inverterInfo;
    int commNum = 0; 	//ͨѶ�ϵ����������
    char curTime[15] = {'\0'};
    unsigned int CurEnergy = 0;
    unsigned int SysPower = 0;
    memcpy(curTime,ecu.curTime,14);
    curTime[14] = '\0';
    if(ecu.validNum > 0)
    {
        client_Data = malloc(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);

        memcpy(client_Data,"APS16AAAAA",10);//
        //�ж������ֻ���
        for(i = 0;i< ecu.validNum; i++)
        {
            if((0 != inverterInfo[i].model)&&(0xff != inverterInfo[i].model))
            {
                systemModel = inverterInfo[i].model;
                break;
            }
        }
        //���ݲ�ͬ������֯����ͷ
        if((systemModel == 0)||(systemModel == 0xff))
        {
            memcpy(&client_Data[10],"0002",4);
        }else
        {
            sprintf(&client_Data[10],"%04d",systemModel);
            printf("systemModel:%d\n",systemModel);
        }

        memcpy(&client_Data[14],"AAA1",4);
        memcpy(&client_Data[18],ecu.ECUID12,12);
        memset(&client_Data[30],'0',63);
        memcpy(&client_Data[93],"END",3);
        length = 96;
        //�ɼ�һ������
        commNum = 0;
        curinverter = inverterInfo;
        rt_enter_critical();
        for(i = 0;i< ecu.validNum; i++)
        {
            //�ɼ�ÿһ���Ż���������
            //�ж������Ƿ����һ���и��£���������ˣ�����Ҫ�ϴ������û���¾Ͳ��ϴ�   ֻ������һ��ͨѶ������һ�βɼ����Ż����
            if(((!memcmp(curinverter->LastCollectTime,"00000000000000",14))&&(memcmp(curinverter->LastCommTime,"00000000000000",14))) || (Time_difference(curinverter->LastCommTime,curinverter->LastCollectTime) > 0))
                //if(1)
            {
                commNum++;
                curinverter->status.comm_status = 1;

                //�ڲ������и���
                //ID	12�ֽ�
                memcpy(&client_Data[length],curinverter->uid,12);
                length+=12;
                //�����ѹ 6�ֽ�
                sprintf(&client_Data[length],"%06d",curinverter->PV_Output*100);
                length += 6;
                //������� 6�ֽ�
                sprintf(&client_Data[length],"%06d",curinverter->PI_Output*100);
                length += 6;
                //������� 6�ֽ�
                sprintf(&client_Data[length],"%06d",curinverter->Power_Output*100);
                length += 6;
                //������� 10�ֽ�
                if(curinverter->Last_PV_Output_Energy > curinverter->PV_Output_Energy)
                {
                    //����һ�ַ�����> ��ǰ�ַ�����
                    //���ֵ���Ϊ�ɼ������������
                    curinverter->EnergyPV_Output= curinverter->PV_Output_Energy;
                    sprintf(&client_Data[length],"%010d",(curinverter->PV_Output_Energy/36));
                    length += 10;
                }else
                {
                    if(0 == curinverter->status.turn_on_collect_data)
                    {
                        curinverter->EnergyPV_Output = 0;
                    }else
                    {
                        curinverter->EnergyPV_Output= (curinverter->PV_Output_Energy - curinverter->Last_PV_Output_Energy);
                    }
                    //pv1�������(���ּ����ֵ)
                    sprintf(&client_Data[length],"%010d",(curinverter->EnergyPV_Output/36));
                    length += 10;
                }
                //�¶� 3�ֽ�
                sprintf(&client_Data[length],"%03d",curinverter->temperature);
                //memcpy(&client_Data[length],"100",3);
                length += 3;
                //Optimizer_pv1  1�ֽ�
                client_Data[length++]  = '1';
                // PV1�����ѹ 6�ֽ�
                sprintf(&client_Data[length],"%06d",curinverter->PV1*100);
                length += 6;
                // pv1������� 6�ֽ�
                sprintf(&client_Data[length],"%06d",curinverter->PI*100);
                length += 6;
                //�����ǰһ�ֵ���С����һ�ֵĵ���  ����Ĭ��Ϊ�������ˣ�����ֱ�ӻ�ȡ������Ϊ��ʱ��˲ʱ����
                if(curinverter->Last_PV1_Energy > curinverter->PV1_Energy)
                {
                    curinverter->AveragePower1 = curinverter->Power1;
                    sprintf(&client_Data[length],"%06d",(curinverter->Power1*100));
                    length += 6;
                    curinverter->EnergyPV1 = curinverter->PV1_Energy;
                    sprintf(&client_Data[length],"%010d",(curinverter->PV1_Energy/36));
                    length += 10;
                }else
                {
                    unsigned short Power1;

                    //����=(��ǰһ�ֵ���-��һ�ֵ���)/ʱ��Ĳ�ֵ
                    if(!memcmp(curinverter->LastCollectTime,"00000000000000",14))
                    {
                        curinverter->AveragePower1 = curinverter->Power1;
                    }else
                    {
                        Power1 = (curinverter->PV1_Energy - curinverter->Last_PV1_Energy)/Time_difference(curinverter->LastCommTime,curinverter->LastCollectTime);
                        curinverter->AveragePower1 = Power1;
                    }

                    //pv1���빦��(����õ�ƽ������)
                    sprintf(&client_Data[length],"%06d",((unsigned short)curinverter->AveragePower1*100));
                    length += 6;
                    if(0 == curinverter->status.turn_on_collect_data)
                    {
                        curinverter->EnergyPV1 = 0;
                    }else
                    {
                        curinverter->EnergyPV1 = (curinverter->PV1_Energy - curinverter->Last_PV1_Energy);
                    }
                    //pv1�������(���ּ����ֵ)
                    sprintf(&client_Data[length],"%010d",(curinverter->EnergyPV1/36));
                    length += 10;
                }

                //Optimizer_pv2  1�ֽ�
                client_Data[length++]  = '2';
                // PV2�����ѹ 6�ֽ�
                sprintf(&client_Data[length],"%06d",curinverter->PV2*100);
                length += 6;
                // pv2������� 6�ֽ�
                sprintf(&client_Data[length],"%06d",curinverter->PI2*100);
                length += 6;
                //�����ǰһ�ֵ���С����һ�ֵĵ���  ����Ĭ��Ϊ�������ˣ�����ֱ�ӻ�ȡ������Ϊ��ʱ��˲ʱ����
                if(curinverter->Last_PV2_Energy > curinverter->PV2_Energy)
                {
                    curinverter->AveragePower2 = curinverter->Power2;
                    sprintf(&client_Data[length],"%06d",(curinverter->Power2*100));
                    length += 6;
                    curinverter->EnergyPV2 = curinverter->PV2_Energy;
                    sprintf(&client_Data[length],"%010d",(curinverter->PV2_Energy/36));
                    length += 10;
                }else
                {
                    unsigned short Power2;
                    //����=(��ǰһ�ֵ���-��һ�ֵ���)/ʱ��Ĳ�ֵ
                    if(!memcmp(curinverter->LastCollectTime,"00000000000000",14))
                    {
                        curinverter->AveragePower2 = curinverter->Power2;
                    }else
                    {
                        Power2 = (curinverter->PV2_Energy - curinverter->Last_PV2_Energy)/Time_difference(curinverter->LastCommTime,curinverter->LastCollectTime);
                        curinverter->AveragePower2 = Power2;
                    }

                    //pv2���빦��(����õ�ƽ������)
                    sprintf(&client_Data[length],"%06d",((unsigned short)curinverter->AveragePower2*100));
                    length += 6;
                    if(0 == curinverter->status.turn_on_collect_data)
                    {
                        curinverter->EnergyPV2 = 0;
                        curinverter->status.turn_on_collect_data = 1;
                    }else
                    {
                        curinverter->EnergyPV2 = (curinverter->PV2_Energy - curinverter->Last_PV2_Energy);
                    }

                    //pv2�������(���ּ����ֵ)
                    sprintf(&client_Data[length],"%010d",(curinverter->EnergyPV2/36));
                    length += 10;
                }

                //END
                client_Data[length++] = 'E';
                client_Data[length++] = 'N';
                client_Data[length++] = 'D';
                //��ӡ�����Ϣ
                //inverter_Info(curinverter);

                memcpy(curinverter->LastCollectTime,curinverter->LastCommTime,15);
                curinverter->LastCollectTime[14] = '\0';
                curinverter->Last_PV1_Energy = curinverter->PV1_Energy;
                curinverter->Last_PV2_Energy = curinverter->PV2_Energy;
                curinverter->Last_PV_Output_Energy = curinverter->PV_Output_Energy;
            }
            else
            {
                curinverter->status.comm_status = 0;
            }
            curinverter++;
        }

        ecu.count= commNum;
        //����ECU�����������
        if(commNum > 0)
        {
            char system_power_str[11] = {'\0'};
            char current_energy_str[11] = {'\0'};
            char commNum_str[4] = {'\0'};

            curinverter = inverterInfo;
            for(i = 0;i< ecu.validNum; i++)
            {
                //�����̨�ɼ���ʱ��ɹ�
                if(curinverter->status.comm_status == 1)
                {
                    CurEnergy = CurEnergy + curinverter->EnergyPV1 + curinverter->EnergyPV2;
                    SysPower = SysPower + curinverter->AveragePower1 + curinverter->AveragePower2;
                }
                curinverter++;
            }
            //��ǰһ�����빦��
            sprintf(system_power_str,"%010d",(long)SysPower*100);
            memcpy(&client_Data[30],system_power_str,10);
            //��ǰһ���������
            sprintf(current_energy_str,"%010d",(long)CurEnergy/36);
            memcpy(&client_Data[50],current_energy_str,10);
            memcpy(&client_Data[70],curTime,14);
            //���һ��ͨѶ�ϵĴ���
            ecu.lastCommNum = commNum;
            sprintf(commNum_str,"%03d",commNum);
            memcpy(&client_Data[84],commNum_str,3);

            client_Data[5] = (length/10000)%10 + 0x30;
            client_Data[6] = (length/1000)%10+ 0x30;
            client_Data[7] = (length/100)%10+ 0x30;
            client_Data[8] = (length/10)%10+ 0x30;
            client_Data[9] = (length/1)%10+ 0x30;

        }
        client_Data[length++] = '\0';	//�����ļ���ʱ����ӻ��з����ϴ����ݵ�ʱ������ӻ��з�
        rt_exit_critical();

        ecu.current_energy = (float)CurEnergy/36000000;		//��������ת��Ϊǧ��ʱ
        ecu.system_power = SysPower/10;
        ecu.life_energy = ecu.life_energy + ecu.current_energy;

        printfloatmsg(ECU_DBG_COLLECT,"ecu.current_energy",ecu.current_energy);
        printfloatmsg(ECU_DBG_COLLECT,"ecu.system_power",ecu.system_power);
        printfloatmsg(ECU_DBG_COLLECT,"ecu.life_energy",ecu.life_energy);
        update_life_energy(ecu.life_energy);								//����ϵͳ��ʷ������
        //���汨������
        if(commNum > 0)
        {

            save_system_power(ecu.system_power,curTime);			//����ϵͳ����    	�����´��
            update_daily_energy(ecu.current_energy,curTime);		//����ÿ�շ�����  	��������
            update_monthly_energy(ecu.current_energy,curTime);		//����ÿ�µķ�����	�����´��
            update_yearly_energy(ecu.current_energy,curTime);  		//����ÿ��ķ�����
            //��ౣ�������µ�����
            delete_system_power_2_month_ago(curTime);		//ɾ��������ǰ������
            //�������������������ϴ�

            //save_record(client_Data,curTime);				//������Ҫ���͸��������ı���
            //print2msg(ECU_DBG_COLLECT,"client Data:",client_Data);
            //�������ݵ��ļ��� ������Ϊ���һ�εķ�����
            save_last_collect_info();
            create_alarm_record(inverterInfo);
            //����07����������ݵ��ļ���
            //save_collect_info(curTime);
            //��ౣ��2�������
            //delete_collect_info_2_day_ago(curTime);//ɾ������ǰ������

        }

        if((commNum > 0)||(ecu.thirdCommNum >0))
        {
            protocol_JSON(curTime,thirdInverterInfo,client_Data);
        }

        free(client_Data);
        client_Data = NULL;
    }
}

void Collect_Control_Record(void)
{
    char *control_Data = NULL;
    int length = 0; //��ǰ��������λ��
    int i = 0;				//��ѵ����
    inverter_info *curinverter = inverterInfo;
    int commNum = 0; 	//ͨѶ�ϵ����������
    char curTime[15] = {'\0'};
    apstime(curTime);

    if(ecu.validNum > 0)
    {
        control_Data = malloc(CONTROL_RECORD_HEAD + CONTROL_RECORD_ECU_HEAD + CONTROL_RECORD_INVERTER_LENGTH * MAXINVERTERCOUNT + CONTROL_RECORD_OTHER);
        //ͷ��Ϣ
        memcpy(control_Data,"APS13AAAAAA158AAA1",18);
        //ECUͷ��Ϣ
        memcpy(&control_Data[18],ecu.ECUID12,12);
        memset(&control_Data[30],'0',18);
        memcpy(&control_Data[48],"END",3);
        length = 51;
        //�Ż�����Ϣ
        commNum = 0;
        curinverter = inverterInfo;
        for(i = 0;i< ecu.validNum; i++)
        {
            if(curinverter->status.comm_status == 1)
            {
                commNum++;

                //�ڲ������и���
                //ID	12�ֽ�
                memcpy(&control_Data[length],curinverter->uid,12);
                length += 12;
                sprintf(&control_Data[length],"%05d",curinverter->heart_rate);
                length += 5;
                sprintf(&control_Data[length],"%05d",curinverter->off_times);
                length += 5;
                sprintf(&control_Data[length],"%03d",curinverter->restartNum);
                length += 3;
                sprintf(&control_Data[length],"%03d",curinverter->RSSI);
                length += 3;
                sprintf(&control_Data[length],"%05d",curinverter->PV1_low_voltageNUM);
                length += 5;
                sprintf(&control_Data[length],"%05d",curinverter->PV2_low_voltageNUM);
                length += 5;

                control_Data[length++] = 'E';
                control_Data[length++] = 'N';
                control_Data[length++] = 'D';

            }
            curinverter++;
        }


        //����ECU�����������
        if(commNum > 0)
        {
            char commNum_str[5] = {'\0'};


            memcpy(&control_Data[34],curTime,14);
            //���һ��ͨѶ�ϵĴ���
            ecu.lastCommNum = commNum;
            sprintf(commNum_str,"%04d",commNum);
            memcpy(&control_Data[30],commNum_str,4);

            control_Data[5] = (length/10000)%10 + 0x30;
            control_Data[6] = (length/1000)%10+ 0x30;
            control_Data[7] = (length/100)%10+ 0x30;
            control_Data[8] = (length/10)%10+ 0x30;
            control_Data[9] = (length/1)%10+ 0x30;

        }
        control_Data[length++] = '\0';	//�����ļ���ʱ����ӻ��з����ϴ����ݵ�ʱ������ӻ��з�

        //���汨������
        if(commNum > 0)
        {
            save_control_record(control_Data,curTime);
            //print2msg(ECU_DBG_COLLECT,"client Data:",control_Data);

        }

        free(control_Data);
        control_Data = NULL;
    }

}


int getalldata(void)
{
    int i = 0,ret = 0;
    unsigned short pre_heart_rate = 0;

    for(i=0;i<3;i++)
    {
        if(-1==zb_test_communication())
            zigbee_reset();
        else
            break;
    }
    //�ɼ����ݵ��ṹ����
    for(ecu.curSequence = 0;ecu.curSequence<ecu.validNum;ecu.curSequence++)
    {
        ret = 0;
        for(i=0;i<8;i++)
        {
            pre_heart_rate = inverterInfo[ecu.curSequence].heart_rate;
            ret = zb_query_heart_data(&inverterInfo[ecu.curSequence]);
            if(ret == 1)
            {
                if(inverterInfo[ecu.curSequence].heart_rate < pre_heart_rate)
                {
                    //��ǰһ����������+1
                    if(inverterInfo[ecu.curSequence].restartNum < 255)
                        inverterInfo[ecu.curSequence].restartNum++;
                }
                break;
            }
        }
        if(ret != 1)
        {
            inverterInfo[ecu.curSequence].status.collect_ret = 0;
            comm_failed_Num += 7;
            if(comm_failed_Num > (ecu.validNum * 7 *12))
            {
                for(ecu.curSequence = 0;ecu.curSequence < ecu.validNum;ecu.curSequence++)
                {
                    inverterInfo[ecu.curSequence].restartNum = 0;
                }
                comm_failed_Num = 0;
                ecu.curSequence = 0;
            }
        }else{
            inverterInfo[ecu.curSequence].status.collect_ret = 1;
        }
    }
    ecu.polling_total_times++;
    return 0;
}

int displayonPhone(void)
{
    for(ecu.curSequence = 0;ecu.curSequence<ecu.validNum;ecu.curSequence++)
    {
        if(0 == inverterInfo[ecu.curSequence].status.collect_ret)
        {
            inverterInfo[ecu.curSequence].status.comm_failed3_status = 0;
            inverterInfo[ecu.curSequence].no_getdata_num++;
            if(inverterInfo[ecu.curSequence].no_getdata_num>254)
            {
                inverterInfo[ecu.curSequence].no_getdata_num = 254;
            }

        }else{
            inverterInfo[ecu.curSequence].status.comm_failed3_status = 1;
            inverterInfo[ecu.curSequence].no_getdata_num = 0;
        }

    }
    memcpy(ecu.JsonTime,ecu.curTime,15);
    return 0;
}

int process_Heart(void)
{
    //ÿ1.5S����һ������
    if(COMM_Timeout_Event == 1)
    {
        if(ecu.curHeartSequence >= ecu.validNum)
        {
            ecu.curHeartSequence = 0;
        }
        zb_sendHeart(inverterInfo[ecu.curHeartSequence].uid);
        ecu.curHeartSequence++;
        COMM_Timeout_Event = 0;
    }
    return 0;
}

int process_IDUpdate(void)
{
    if(1 == ecu.idUpdateFlag)
    {
        updateID();
        ecu.idUpdateFlag = 0;
    }
    if(1 == ecu.ThirdIDUpdateFlag)
    {
        updateThirdID();
        ecu.ThirdIDUpdateFlag = 0;
    }
    return 0;
}

//���߳���Ҫ����������ݵĲɼ�����
void ECUCollect_thread_entry(void* parameter)
{
    int CollectClientThistime=0, CollectClientDurabletime=65535, CollectClientReportinterval=300;			//�ɼ��������ʱ�����
    int CollectControlThistime=0, CollectControlDurabletime=65535, CollectControlReportinterval=900;	//�ɼ�Զ�̿�������ʱ�����
    int cur_time_hour;
    ECUCommThreadFlag = EN_ECUHEART_DISABLE;
    ecu.curHeartSequence = 0;
    init_all(inverterInfo); //��ʼ�����������
    TrinaFunction();
    rt_thread_delay(RT_TICK_PER_SECOND * START_TIME_COLLECT);
    
    while(1)
    {
        if(compareTime(CollectClientDurabletime ,CollectClientThistime,CollectClientReportinterval))
        {
            //5���Ӳɼ���صķ���������
            CollectClientThistime = acquire_time();
            apstime(ecu.curTime);
            cur_time_hour = get_hour();
            if(ecu.validNum >0)
            {
                ECUCommThreadFlag = EN_ECUHEART_DISABLE;
                getalldata();
                ECUCommThreadFlag = EN_ECUHEART_ENABLE;
                getAllThirdInverterData();	//��ȡ���������������
                optimizeFileSystem(300);
                printmsg(ECU_DBG_COLLECT,"Collect DATA Start");
                //�ɼ�ʵʱ����
                Collect_Client_Record();

                //�����ֻ���ʾ
                displayonPhone();
                //��϶������ݲɼ�
                Collect_TrinaSolar_Record();
                printmsg(ECU_DBG_COLLECT,"Collect DATA End");
                process_rsd_single();//���RSD�����Ƿ���ȷ
                process_IDUpdate();//���ID����Ƿ���Ҫ����


                remote_update(inverterInfo);
                if((cur_time_hour>9)&&(1 == ecu.flag_ten_clock_getshortaddr))
                {
                    get_inverter_shortaddress(inverterInfo);
                    if(ecu.polling_total_times>3)
                    {
                        ecu.flag_ten_clock_getshortaddr = 0;							//ÿ��10��ִ�������»�ȡ�̵�ַ���־λ��Ϊ0
                    }
                }

                //������ѵû�����ݵ�������������»�ȡ�̵�ַ����
                bind_nodata_inverter(inverterInfo);
                ECUCommThreadFlag = EN_ECUHEART_DISABLE;

            }
             process_ZigBeeTransimission();
        }
        if((CollectClientDurabletime-CollectClientThistime)<=305)
            CollectClientReportinterval = 300;
        else if((CollectClientDurabletime-CollectClientThistime)<=600)
            CollectClientReportinterval = 600;
        else
            CollectClientReportinterval = 900;

        rt_thread_delay(RT_TICK_PER_SECOND/10);
        if(compareTime(CollectControlDurabletime ,CollectControlThistime,CollectControlReportinterval))
        {

            if(	ecu.validNum >0)
            {
                ECUCommThreadFlag = EN_ECUHEART_ENABLE;
                optimizeFileSystem(300);
                //�ɼ��������Զ�̿�������
                printmsg(ECU_DBG_COLLECT,"Collect Control DATA  Start");
                CollectControlThistime = acquire_time();
                //�ɼ�Զ�̿�������
                Collect_Control_Record();
                printmsg(ECU_DBG_COLLECT,"Collect Control DATA  End");
                ECUCommThreadFlag = EN_ECUHEART_DISABLE;
            }


        }
        process_Heart();

        CollectClientDurabletime = acquire_time();
        CollectControlDurabletime = acquire_time();
    }
}
