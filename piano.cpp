#include <SFML/System.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <string>
#include <wiringPi.h>
#include <mcp23017.h>

using namespace std;

short gpios[26] = {7, 15, 16, 0, 1, 2, 3, 4, 5, 12, 13, 6, 14, 10, 11, 30, 31, 21, 22, 26, 23, 24, 27, 25, 28, 29};
long pressTime[22][4];
short playing[22][4];

sf::SoundBuffer buffers[30];
sf::Sound sounds[30];

void setup();
void audio();

int main() {
	setup();

	sf::Thread thread(&audio);
	thread.launch();

	struct timespec time;

	while(1) {
		for(int i = 0 ; i < 8 ; i++) {
			digitalWrite(100+i, HIGH);
			for(int j = 0 ; j < 1 ; j++) {
				if(digitalRead(gpios[j])) {
                    if (i < 4) {
						if (pressTime[j][i] == -1) {
							clock_gettime(CLOCK_MONOTONIC, &time);
							pressTime[j][i] = time.tv_nsec;
						}
					} else if (!playing[j][i - 4]) {
						playing[j][i - 4] = 1;
					}
                } else {
					if(i<4) pressTime[j][i] = -1;
					else playing[j][i-4] = 0;
				}
			}
			digitalWrite(100+i, LOW);
		}
	}
	
	return 0;
}

void setup() {
	wiringPiSetup();
	mcp23017Setup (100, 0x20);

	for(int i = 0 ; i < 8 ; i++) {
		pinMode(100+i, OUTPUT);
		digitalWrite(100+i, LOW);
	}

	for(int i = 0 ; i < 23 ; i++) {
		pinMode(gpios[i], INPUT);
	}

	for(int i = 0 ; i < 22 ; i++) {
		for(int j = 0 ; j < 4 ; j++) {
			pressTime[j][i] = -1;
		}
	}

	for(int i = 0 ; i < 22 ; i++) {
		for(int j = 0 ; j < 4 ; j++) {
			playing[j][i] = 0;
		}
	}

	string notes[30] = {
		"A0","A1","A2","A3","A4","A5","A6","A7",
		"C1","C2","C3","C4","C5","C6","C7","C8",
		"D#1","D#2","D#3","D#4","D#5","D#6","D#7",
		"F#1","F#2","F#3","F#4","F#5","F#6","F#7",
	};

	for(int i = 0 ; i < 30 ; i++) {
		if(! buffers[i].loadFromFile("samples/"+notes[i]+".wav") )
			exit(0);
		sounds[i].setBuffer(buffers[i]);
	}
}

typedef struct infoNote infoNote;

struct infoNote {
	int num;
	float pitch;
};

void initInfosNotes(infoNote**);

void audio() {

	infoNote** infosNotes;
	initInfosNotes(infosNotes);

	while(1) {
		for(int i = 0 ; i < 22 ; i++) {
			for(int j = 0 ; j < 4 ; j++) {
				if(playing[i][j] == 1) {
					playing[j][i] = 2;
					sounds[infosNotes[i][j].num].setPitch(infosNotes[i][j].pitch);
					sounds[infosNotes[i][j].num].play();
				} else if(!playing[i][j] && sounds[infosNotes[i][j].num].getStatus() == sf::SoundSource::Status::Playing) {
					sounds[infosNotes[i][j].num].stop();
				}
			}
		}
	}
}

void initInfosNotes(infoNote** infosNotes) {
	
	infosNotes = (infoNote**) malloc(sizeof(infoNote*)*22);

	float prevPitch = 1 / pow(2, 1/12);
	float nextPitch = pow(2, 1/12);

	for(int i = 0 ; i <= 18 ; i+=3) {
		
		infosNotes[i] = (infoNote*) malloc(sizeof(infoNote)*4);
		infosNotes[i+1] = (infoNote*) malloc(sizeof(infoNote)*4);
		infosNotes[i+2] = (infoNote*) malloc(sizeof(infoNote)*4);
		
		infosNotes[i][0].num = i/3;
		infosNotes[i][1].num = i/3;
		infosNotes[i][2].num = 8 + i/3;
		infosNotes[i][3].num = 8 + i/3;

		infosNotes[i+1][0].num = 8 + i/3;
		infosNotes[i+1][1].num = 16 + i/3;
		infosNotes[i+1][2].num = 16 + i/3;
		infosNotes[i+1][3].num = 16 + i/3;

		infosNotes[i+2][0].num = 23 + i/3;
		infosNotes[i+2][1].num = 23 + i/3;
		infosNotes[i+2][2].num = 23 + i/3;
		infosNotes[i+2][3].num = 1 + i/3;


		infosNotes[i][0].pitch = 1;
		infosNotes[i][1].pitch = nextPitch;
		infosNotes[i][2].pitch = prevPitch;
		infosNotes[i][3].pitch = 1;

		infosNotes[i+1][0].pitch = nextPitch;
		infosNotes[i+1][1].pitch = prevPitch;
		infosNotes[i+1][2].pitch = 1;
		infosNotes[i+1][3].pitch = nextPitch;

		infosNotes[i+2][0].pitch = prevPitch;
		infosNotes[i+2][1].pitch = 1;
		infosNotes[i+2][2].pitch = nextPitch;
		infosNotes[i+2][3].pitch = prevPitch;
	}

	infosNotes[21][0].num = 7;
	infosNotes[21][1].num = 7;
	infosNotes[21][2].num = 15;
	infosNotes[21][3].num = 15;

	infosNotes[21][0].pitch = 1;
	infosNotes[21][1].pitch = nextPitch;
	infosNotes[21][2].pitch = prevPitch;
	infosNotes[21][3].pitch = 1;
}
