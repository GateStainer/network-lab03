#ifndef TCP_H_
#define TCP_H_

#define SERV_PORT 4321
#define SERV_IP "114.212.191.33"

#pragma pack(1)

struct weather_temperature
{
	uint8_t weather;
	uint8_t temperature;
};
typedef struct weather_temperature WT;
struct weatherInfo
{
	unsigned short year;
	uint8_t month;
	uint8_t day;
	uint8_t num;
};

typedef struct weatherInfo weatherInfo;

#endif

