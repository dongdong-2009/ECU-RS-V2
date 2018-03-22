#ifndef __VERSION_H__
#define	__VERSION_H__
//版本号
#define ECU_VERSION 		"ECU_R_RS"
#define ECU_EMA_VERSION		"S"
#define MAJORVERSION  		"1"
#define MINORVERSION		"1.1"


#define INTERNAL_TEST_VERSION		0	//内部版本号，在内部测试过程中使用

#define ECU_VERSION_LENGTH (strlen(ECU_VERSION)+strlen(MAJORVERSION)+strlen(MINORVERSION) + 2)
//区域
#define ECU_AREA  			""

#endif /*__VERSION_H__*/
