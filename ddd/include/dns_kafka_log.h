#include "rdkafka.h"  /* for Kafka driver */
#include "rd.h"
#include "rdtime.h"

#define SAMPLE_BROKERS "10.11.255.248:9092,10.11.255.249:9092,10.11.255.250:9092,10.11.255.251:9092,10.11.255.252:9092,10.11.255.254:9092"
#define	SAMPLE_TOPIC	"test"

#define	QUEUE_BUF_MAX_MSG	"100000"
#define	QUEUE_BUF_MAX_MSG_INT	(100000)

/***********************************************/
//function:				kafka_log_init
//description:			初始化kafka client
//arguments:
//		brokers:		kafka服务器的连接地址
//
//return:
//						成功：rd_kafka_t指针
//						失败：NULL指针
/***********************************************/
rd_kafka_t * kafka_log_init(char *brokers);



/***********************************************/
//function:				kafka_log_set_topic
//description:			设置日志发送的topic
//arguments:
//		rk:		由kafka_log_init返回的指针
//		topic:	topic的名称
//
//return:
//						成功：rd_kafka_topic_t指针
//						失败：NULL指针
/***********************************************/
rd_kafka_topic_t * kafka_log_set_topic(rd_kafka_t * rk,char*topic);


/***********************************************/
//function:				kafka_log_send
//description:			发送一条日志
//arguments:
//		rkt:	由kafka_log_set_topic返回的指针
//		msg:	日志内容
//		msglen:	日志内容长度
//
//return:
//						成功：0
//						失败：-1
/***********************************************/
int kafka_log_send(rd_kafka_topic_t *rkt,char*msg,uint msglen);
