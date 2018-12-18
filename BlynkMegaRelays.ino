#include <ArduinoJson.h>
#include <EEPROM.h>
#define BLYNK_PRINT Serial
#include <SPI.h>
#include <Ethernet.h>
#include <BlynkSimpleEthernet.h>


#define LED_STT	LED_BUILTIN
#define RELAY1	A8
#define RELAY2	A9
#define RELAY3	A10
#define RELAY4	A11
#define RELAY5	A12
#define RELAY6	A13
#define RELAY7	A14
#define RELAY8	A15
#define RELAY9	28
#define RELAY10	30
#define RELAY11	32
#define RELAY12	34
#define RELAY13	36
#define RELAY14	38
#define RELAY15	40
#define RELAY16	42
#define RELAY17	21
#define RELAY18	20
#define RELAY19	19
#define RELAY20	18
#define RELAY21	17
#define RELAY22	16
#define RELAY23	15
#define RELAY24	14

int RL[] =
{
	LED_STT,
	RELAY1,
	RELAY2,
	RELAY3,
	RELAY4,
	RELAY5,
	RELAY6,
	RELAY7,
	RELAY8,
	RELAY9,
	RELAY10,
	RELAY11,
	RELAY12,
	RELAY13,
	RELAY14,
	RELAY15,
	RELAY16,
	RELAY17,
	RELAY18,
	RELAY19,
	RELAY20,
	RELAY21,
	RELAY22,
	RELAY23,
	RELAY24
};


#define	_version_ ("v0.1")

#define DB(x)	Serial.println(x)
#define Db(x)	Serial.print(x)
#define DbTime()	{ Db(F("[")); Db(millis()); DB(F("]")); }

#define GITH_SERVER "autominer.xyz"
byte mac[6];// = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // max address for 
//byte ip[4];// = { 10,210,6,237 }; // ip address for ethernet shield
String hid;	//hub id, device id

String BLYNK_AUTH;
String BLYNK_SERVER = GITH_SERVER;
uint16_t BLYNK_PORT = 8442;

#define TERMINAL_PIN	V0
WidgetTerminal Terminal(TERMINAL_PIN);

#pragma region GET_DATA_FROM_EEPROM
/*
getMacAdd
	|---- byte 0..5: mac
	|---- byte 6..7: flag Ok
*/
String getMacAddr(bool forceGenerate = false) {
	//byte 0-5: mac
	//byte 6-7: flag Ok

	if (forceGenerate) {
		goto GENERATE;
	}

	if ((EEPROM.read(6) == 'O') && (EEPROM.read(7) == 'k')) {
		String _mac;
		for (int i = 0; i < 6; i++) {
			mac[i] = EEPROM.read(i);
			//_mac += mac[i] <= 0xf ? "0" + String(mac[i], HEX) : String(mac[i], HEX);
			_mac += String(mac[i], HEX);
		}
		String _id = _mac.substring(_mac.length() - 6);
		Db(F("MAC: "));
		DB(_mac);
		Db(F("HID: "));
		DB(_id);
		return _id;
	}
	else {
	GENERATE:
		Db(F("Generate "));
		randomSeed(analogRead(A0));
		EEPROM.write(0, 0x00);				delay(10);
		EEPROM.write(1, 0x16);				delay(10);
		EEPROM.write(2, 0x3e);				delay(10);
		EEPROM.write(3, random(0, 0x7f));	delay(10);
		EEPROM.write(4, random(0, 0xff));	delay(10);
		EEPROM.write(5, random(0, 0xff));	delay(10);

		EEPROM.write(6, 'O');
		delay(10);
		EEPROM.write(7, 'k');
		delay(10);
		return getMacAddr();
	}
}

#pragma endregion


#pragma region HTTP
EthernetClient http_client;

const char* server = GITH_SERVER;  //gith.cf <- server's address || gith-server.herokuapp.com
const unsigned long HTTP_TIMEOUT = 5000;  // max respone time from server

String getBlynkAuth() {
	String _auth;
	String _domain;
	int _port;
START_GET:
	if (connect(server)) {
		String resource = "/db-get/" + hid + "/blynk?token=02df7e73987413d859dead7731c0cf06";
		DB(String(server) + "/db-get/" + hid + "/blynk?token=***");
		if (sendRequest(server, resource.c_str()) && skipResponseHeaders()) {
			http_client.setTimeout(1000);
			while (http_client.available() > 0) {
				String _blynkInfo = http_client.readString();

				DB(_blynkInfo);

				DynamicJsonBuffer jsonBuffer(200);
				JsonObject& blynk = jsonBuffer.parseObject(_blynkInfo);

				//blynk.prettyPrintTo(Serial);

				_auth = blynk["auth"].as<String>();
				_domain = blynk["server"].as<String>();
				_port = blynk["port"].as<int>();

				if (_auth.length() < 3) {
					_auth = "null";
				}
				else {
					BLYNK_AUTH = _auth;
				}
				if (_domain.length() > 2) {
					BLYNK_SERVER = _domain;
				}
				if (_port > 0) {
					BLYNK_PORT = _port;
				}
				Db(F("Auth: ")); DB(BLYNK_AUTH);
				Db(F("Domain: ")); DB(BLYNK_SERVER);
				Db(F("Port: ")); DB(BLYNK_PORT);
			}
			DB(_auth);
		}
		else {
			http_client.stop();
			DB(F("Try again"));
			delay(1000);
			goto START_GET;
		}
	}
	else {
		http_client.stop();
		DB(F("Try again"));
		delay(1000);
		goto START_GET;
	}
	http_client.stop();
	if (_auth.length() > 5) {
		return _auth.substring(1, _auth.length() - 1);
	}
	else if (_auth == "null") {
		unsigned long i = 0;
		while (true)
		{
			Db(F("["));
			Db(i++);
			DB(F("]"));
			Db(F("Device ID: ")); DB(hid);
			if (i % 2) {
				DB(F("Thiet bi nay chua duoc dang ki, hay lien he voi tac gia! \n0387845097\nfacebook.com/tackelua"));
			}
			else {
				DB(F("This device has not been registered, please contact the author!"));
			}
			DB(F("   > > > > > >   facebook.com/tackelua   < < < < < <  "));
			DB(F("\r\n#\r\n#\r\n#\r\n"));

#pragma region serial handle
			delay(100);
			serial_handle();
			delay(200);
			serial_handle();
			delay(300);
			serial_handle();
			delay(300);
			serial_handle();
			delay(200);
			serial_handle();
			delay(100);
			serial_handle();
#pragma endregion

			goto START_GET;
		}
	}
	else return "";
}

#pragma region http helper
bool connect(const char* hostName) {
	Db(F("["));
	Db(millis());
	DB(F("]\r\nConnect to server"));
	//DB(hostName);

	bool ok = http_client.connect(hostName, 80);

	ok ? DB(F("Connected")) : DB(F("Connection Failed!"));
	return ok;
}

// Send the HTTP GET request to the server
bool sendRequest(const char* host, const char* resource) {
	//Db("GET ");
	//DB(resource);

	http_client.print("GET ");
	http_client.print(resource);
	http_client.println(" HTTP/1.0");
	http_client.print("Host: ");
	http_client.println(host);
	http_client.println("Connection: close");
	http_client.println();

	return true;
}

// Skip HTTP headers so that we are at the beginning of the response's body
bool skipResponseHeaders() {
	// HTTP headers end with an empty line
	char endOfHeaders[] = "\r\n\r\n";

	http_client.setTimeout(HTTP_TIMEOUT);
	bool ok = http_client.find(endOfHeaders);

	if (!ok) {
		DB("No response or invalid response!");
	}

	return ok;
}
#pragma endregion
#pragma endregion

#define LED_ON()	digitalWrite(LED_STT, HIGH)
#define LED_OFF()	digitalWrite(LED_STT, LOW)
void Ethernet_init() {
	DbTime();
	DB(F("Ethernet initializing. Please wait..."));

	while (!Ethernet.begin(mac)) {
		DB(F("Failed to configure Ethernet\r\nTry again"));
		delay(1000);
	}
	//Ethernet.begin(mac, ip);

	Db(F("localIP: "));
	DB(Ethernet.localIP());

	Db(F("gatewayIP: "));
	DB(Ethernet.gatewayIP());

	Db(F("dnsServerIP: "));
	DB(Ethernet.dnsServerIP());
}

void Hardware_init() {
	delay(100);
	Serial.begin(115200);
	Serial.setTimeout(10);

	Db(F("Version: "));
	DB(_version_);
	DB(__TIME__ " " __DATE__);

	DB("Init Relays ");
	for (uint8_t i = 0; i <= 24; i++) {
		pinMode(RL[i], OUTPUT);
		digitalWrite(RL[i], HIGH);
		Db(".");
	}
	DB();
	LED_ON();

	hid = getMacAddr();
}
//===============

//
//BLYNK_WRITE(V1) {
//	digitalWrite(RELAY1, param.asInt());
//}
//BLYNK_WRITE(V2) {
//	digitalWrite(RELAY2, param.asInt());
//}
//BLYNK_WRITE(V3) {
//	digitalWrite(RELAY3, param.asInt());
//}
//BLYNK_WRITE(V4) {
//	digitalWrite(RELAY4, param.asInt());
//}
//BLYNK_WRITE(V5) {
//	digitalWrite(RELAY5, param.asInt());
//}
//BLYNK_WRITE(V6) {
//	digitalWrite(RELAY6, param.asInt());
//}
//BLYNK_WRITE(V7) {
//	digitalWrite(RELAY7, param.asInt());
//}
//BLYNK_WRITE(V8) {
//	digitalWrite(RELAY8, param.asInt());
//}
//BLYNK_WRITE(V9) {
//	digitalWrite(RELAY9, param.asInt());
//}
//BLYNK_WRITE(V10) {
//	digitalWrite(RELAY10, param.asInt());
//}
//BLYNK_WRITE(V11) {
//	digitalWrite(RELAY11, param.asInt());
//}
//BLYNK_WRITE(V12) {
//	digitalWrite(RELAY12, param.asInt());
//}
//BLYNK_WRITE(V13) {
//	digitalWrite(RELAY13, param.asInt());
//}
//BLYNK_WRITE(V14) {
//	digitalWrite(RELAY14, param.asInt());
//}
//BLYNK_WRITE(V15) {
//	digitalWrite(RELAY15, param.asInt());
//}
//BLYNK_WRITE(V16) {
//	digitalWrite(RELAY16, param.asInt());
//}


void command_handle(String& cmd) {

}
void serial_handle() {
	if (Serial.available() > 0) {
		String input = Serial.readString();
		Db(F(">Serial: ")); DB(input);
		if (isAlphaNumeric(input[0]) || (input[0] == '/')) {
			command_handle(input);
			return;
		}
	}
}

#pragma region BLYNK
void waitBlynkConnect() {
	while (!Blynk.connect()) {
		Blynk.run();
		//Blynk.connect();
		delay(10);
	}
	Blynk_run(3);
}
void Blynk_run(int times) {
	//Db(F("Blynk run "));
	//DB(times);
	//unsigned long t = millis();
	Blynk.run();	delay(1);
	if (times > 1) {
		for (int i = 1; i < times; i++) {
			Blynk.run();
			//delay(1);
			serial_handle();
		}
	}
	//Db(F("Blynk run "));
	//Db(times);
	//Db(F(": "));
	//DB(millis() - t);
}
void getAccountBlynk() {
	DB(F("Get Account Blynk"));
	while (true) {
		getBlynkAuth();
		if (BLYNK_AUTH == "") {
			DB(F("Failed. Try again"));
			delay(1000);
		}
		else {
			break;
		}
	}
}
void Blynk_init() {
	getAccountBlynk();

	//begin blynk
	Blynk.config(BLYNK_AUTH.c_str(), BLYNK_SERVER.c_str(), BLYNK_PORT);

	waitBlynkConnect();

	IPAddress IP = Ethernet.localIP();
	Terminal.print(F("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n"));		Terminal.flush();
	Terminal.print(F("Gith System\r\nAutomatic Miners Manager"));		Terminal.flush();
	Terminal.println();													Terminal.flush();
	Blynk_run(10);

	Terminal.print(F("Version: "));										Terminal.flush();
	Terminal.println(_version_);										Terminal.flush();
	Terminal.print(F("Device IP: "));									Terminal.flush();
	Terminal.println(IP);												Terminal.flush();
	Blynk_run(10);

	Terminal.println(F("--"));
	Terminal.println(F("tackelua@gmail.com"));							Terminal.flush();
	Terminal.println(F("facebook.com/tackelua"));						Terminal.flush();
	//Terminal.println(F("/help to see commands"));						Terminal.flush();
	Terminal.println();													Terminal.flush();
	Blynk_run(10);

	Blynk_run(3);
}

BLYNK_CONNECTED() {


}

BLYNK_WRITE(TERMINAL_PIN) {
	String userTerminal = param.asStr();
	userTerminal.trim();
	if (isAlphaNumeric(userTerminal[0]) || (userTerminal[0] == '/')) {
		Db(F("> Terminal: ")); DB(userTerminal);
		command_handle(userTerminal);
		return;
	}
}

#define BLYNK_WRITE_RL(x);								\
BLYNK_WRITE(x){											\
	LED_ON();											\
	digitalWrite(RL[x], !(bool(param.asInt())));		\
	LED_OFF();											\
}

BLYNK_WRITE_RL(V1);
BLYNK_WRITE_RL(V2);
BLYNK_WRITE_RL(V3);
BLYNK_WRITE_RL(V4);
BLYNK_WRITE_RL(V5);
BLYNK_WRITE_RL(V6);
BLYNK_WRITE_RL(V7);
BLYNK_WRITE_RL(V8);
BLYNK_WRITE_RL(V9);
BLYNK_WRITE_RL(V10);
BLYNK_WRITE_RL(V11);
BLYNK_WRITE_RL(V12);
BLYNK_WRITE_RL(V13);
BLYNK_WRITE_RL(V14);
BLYNK_WRITE_RL(V15);
BLYNK_WRITE_RL(V16);
BLYNK_WRITE_RL(V17);
BLYNK_WRITE_RL(V18);
BLYNK_WRITE_RL(V19);
BLYNK_WRITE_RL(V20);
BLYNK_WRITE_RL(V21);
BLYNK_WRITE_RL(V22);
BLYNK_WRITE_RL(V23);
BLYNK_WRITE_RL(V24);


#pragma endregion



void setup()
{
	Serial.begin(115200);
	Hardware_init();
	Ethernet_init();
	Blynk_init();

	DB(F("System running"));
	LED_OFF();

}

void loop()
{
	Blynk.run();
}
