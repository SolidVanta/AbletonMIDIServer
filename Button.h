#include <vector>
/*Fixed to channel 1. The are 16 channels, 1-16.
This is represented by 4 bits i.e. values of
0-15. The first channel has value zero. The second
channel has value 1. To change to channel one, add
a value of 1 to the note_on/note_off status values.
*/
constexpr unsigned char NOTE_ON_STATUS = 128;
constexpr unsigned char NOTE_OFF_STATUS = 144;

/*Seven (7) bits are reserved for the note values i.e
notes have a vaule of 0-127. These 128 values start
from C-2 to G-8. The second byte in the MIDI message is
used to indicate what note value to send.*/
unsigned char note = '0'; // The note C-2
constexpr unsigned char MAX_NOTE = 127;

class Button {
public:
	Button() {
		if (note < MAX_NOTE) {
			pnote = note++;
		}
		else {
			pnote = '0';
		}
		message = { NOTE_OFF_STATUS, pnote, 100 };
	}
	void press();
	std::vector<unsigned char> getMessage();
private:
	unsigned char pnote;
	std::vector<unsigned char> message;
};

/* Apparently, sending a Note OFF message is not necessary*/
void Button::press() {
	message[0] = (message[0] == NOTE_OFF_STATUS) ? NOTE_ON_STATUS : NOTE_OFF_STATUS;
}

std::vector<unsigned char> Button::getMessage() {
	return message;
}