#include <stdint.h>
#include <string.h>

class SmartCard {
public:
  void process(uint8_t* apdu, size_t apdu_len) {
    // TODO process APDU
  }
};

class ALPAR {
  typedef enum alpar_state {
    ALPAR_STATE_HEADER_0,
    ALPAR_STATE_HEADER_1,
    ALPAR_STATE_HEADER_2,
    ALPAR_STATE_HEADER_3,
    ALPAR_STATE_BODY,
    ALPAR_STATE_LRC,
  } alpar_state_t;

  typedef enum alpar_command {
    ALPAR_COMMAND_INVALID = -1,
    ALPAR_COMMAND_CARD_COMMAND = 0x00,
    ALPAR_COMMAND_PROCESS_T1_COMMAND = 0x01,
    ALPAR_COMMAND_WRITE_I2C = 0x02,
    ALPAR_COMMAND_READ_S9 = 0x03,
    ALPAR_COMMAND_READ_S9_PROTECTION = 0x04,
    ALPAR_COMMAND_WRITE_S9_PROTECTED = 0x05,
    ALPAR_COMMAND_WRITE_S9_UNPROTECTED = 0x06,
    ALPAR_COMMAND_VERIFY_PIN_S9 = 0x07,
    ALPAR_COMMAND_COMPARE_S9 = 0x08,
    ALPAR_COMMAND_CHECK_PRESENCE_CARD = 0x09,
    ALPAR_COMMAND_SEND_NUM_MASK = 0x0A,
    ALPAR_COMMAND_SET_CARD_BAUD_RATE = 0x0B,
    ALPAR_COMMAND_IFSD_REQUEST = 0x0C,
    ALPAR_COMMAND_SET_SERIAL_BAUD_RATE = 0x0D,
    ALPAR_COMMAND_NEGOTIATE_PPS = 0x10,
    ALPAR_COMMAND_SET_CLOCK_CARD = 0x11,
    ALPAR_COMMAND_READ_I2C = 0x12,
    ALPAR_COMMAND_READ_I2C_EXTENDED = 0x13,
    ALPAR_COMMAND_READ_CURRENT_I2C = 0x23,
    ALPAR_COMMAND_POWER_OFF = 0x4D,
    ALPAR_COMMAND_POWER_UP_ISO = 0x69,
    ALPAR_COMMAND_POWER_UP_S9 = 0x6B,
    ALPAR_COMMAND_POWER_UP_I2C = 0x6C,
    ALPAR_COMMAND_POWER_UP_1V8 = 0x68,
    ALPAR_COMMAND_POWER_UP_3V = 0x6D,
    ALPAR_COMMAND_POWER_UP_5V = 0x6E,
    ALPAR_COMMAND_IDLE_MODE_CLOCK_STOP_LOW = 0xA2,
    ALPAR_COMMAND_POWER_DOWN_MODE = 0xA3,
    ALPAR_COMMAND_IDLE_MODE_CLOCK_STOP_HIGH = 0xA4,
    ALPAR_COMMAND_SET_NAD = 0xA5,
    ALPAR_COMMAND_GET_CARD_PARAM = 0xA6,
    ALPAR_COMMAND_GET_READER_STATUS = 0xAA,
    ALPAR_COMMAND_POWER_UP_S10 = 0xC1,
    ALPAR_COMMAND_PROCESS_S10 = 0xC2,
    ALPAR_COMMAND_READ_IO = 0xCE,
    ALPAR_COMMAND_SET_IO = 0xCF,
  } alpar_command_t;

private:
  alpar_state_t state = ALPAR_STATE_HEADER_0;
  size_t length = 0;
  alpar_command_t command = ALPAR_COMMAND_INVALID;
  int buffer_idx = 0;
  int lrc = 0;
  uint8_t buffer[65536];
  SmartCard& smartcard;

public:
  ALPAR(SmartCard& smartcard)
    : smartcard(smartcard) {
    this->state = ALPAR_STATE_HEADER_0;
    this->length = 0;
    this->command = ALPAR_COMMAND_INVALID;
    this->buffer_idx = 0;
    this->lrc = 0;
    memset(this->buffer, 0, sizeof(this->buffer));
  }

  void process(int chr) {
    // ALPAR APDU structure
    // -----------------------
    // | header | body | LRC |
    // -----------------------
    switch (state) {
      // first byte of the header is 0x60
      case ALPAR_STATE_HEADER_0:
        lrc = 0;
        if (chr == 0x60) {
          length = 0;
          command = ALPAR_COMMAND_INVALID;
          state = ALPAR_STATE_HEADER_1;
          lrc ^= (chr & 0xff);
        } else {
          this->clear();
        }
        break;

      // second byte of the header is most significant byte of body length
      case ALPAR_STATE_HEADER_1:
        length = (chr & 0xff) << 8;
        state = ALPAR_STATE_HEADER_2;
        lrc ^= (chr & 0xff);
        break;

      // third byte of the header is the least significant byte of the body length
      case ALPAR_STATE_HEADER_2:
        length = length | (chr & 0xff);
        state = ALPAR_STATE_HEADER_3;
        lrc ^= (chr & 0xff);
        break;

      // fourth byte of the header is the ALPAR command
      case ALPAR_STATE_HEADER_3:
        switch (chr) {
          case ALPAR_COMMAND_CARD_COMMAND:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_CARD_COMMAND;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_PROCESS_T1_COMMAND:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_PROCESS_T1_COMMAND;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_WRITE_I2C:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_WRITE_I2C;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_READ_S9:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_READ_S9;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_READ_S9_PROTECTION:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_READ_S9_PROTECTION;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_WRITE_S9_PROTECTED:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_WRITE_S9_PROTECTED;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_WRITE_S9_UNPROTECTED:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_WRITE_S9_UNPROTECTED;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_VERIFY_PIN_S9:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_VERIFY_PIN_S9;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_COMPARE_S9:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_COMPARE_S9;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_CHECK_PRESENCE_CARD:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_CHECK_PRESENCE_CARD;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_SEND_NUM_MASK:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_SEND_NUM_MASK;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_SET_CARD_BAUD_RATE:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_SET_CARD_BAUD_RATE;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_IFSD_REQUEST:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_IFSD_REQUEST;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_SET_SERIAL_BAUD_RATE:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_SET_SERIAL_BAUD_RATE;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_NEGOTIATE_PPS:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_NEGOTIATE_PPS;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_SET_CLOCK_CARD:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_SET_CLOCK_CARD;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_READ_I2C:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_READ_I2C;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_READ_I2C_EXTENDED:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_READ_I2C_EXTENDED;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_READ_CURRENT_I2C:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_READ_CURRENT_I2C;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_POWER_OFF:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_POWER_OFF;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_POWER_UP_ISO:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_POWER_UP_ISO;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_POWER_UP_S9:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_POWER_UP_S9;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_POWER_UP_I2C:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_POWER_UP_I2C;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_POWER_UP_1V8:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_POWER_UP_1V8;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_POWER_UP_3V:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_POWER_UP_3V;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_POWER_UP_5V:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_POWER_UP_5V;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_IDLE_MODE_CLOCK_STOP_LOW:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_IDLE_MODE_CLOCK_STOP_LOW;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_POWER_DOWN_MODE:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_POWER_DOWN_MODE;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_IDLE_MODE_CLOCK_STOP_HIGH:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_IDLE_MODE_CLOCK_STOP_HIGH;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_SET_NAD:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_SET_NAD;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_GET_CARD_PARAM:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_GET_CARD_PARAM;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_GET_READER_STATUS:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_GET_READER_STATUS;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_POWER_UP_S10:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_POWER_UP_S10;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_PROCESS_S10:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_PROCESS_S10;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_READ_IO:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_READ_IO;
            lrc ^= (chr & 0xff);
            break;
          case ALPAR_COMMAND_SET_IO:
            state = ALPAR_STATE_BODY;
            command = ALPAR_COMMAND_SET_IO;
            lrc ^= (chr & 0xff);
            break;

            // invalid command clear up
          default:
            this->clear();
            break;
        }
        break;
      case ALPAR_STATE_BODY:
        if (length > 0) {
          buffer[buffer_idx] = chr & 0xff;
          length--;
          buffer_idx++;
          lrc ^= (chr & 0xff);
        }

        state = length > 0 ? ALPAR_STATE_BODY : ALPAR_STATE_LRC;
        break;

      case ALPAR_STATE_LRC:
        lrc ^= (chr & 0xff);
        if (lrc == 0) {
          this->smartcard.process(this->buffer, (size_t)this->buffer_idx);
        } else {
          // invalid payload
          uint8_t nack[6] = { 0xE0, 0x00, 0x01, 0x00, 0xff, 0x00 };

          nack[3] = command & 0xff;                                            // set command
          nack[5] = (nack[0] ^ nack[1] ^ nack[2] ^ nack[3] ^ nack[4]) & 0xff;  // set LRC
          Serial.write(nack, sizeof(nack));
        }
        break;

        // default state clear up
      default:
        this->clear();
        break;
    }
  }

  void clear() {
    this->length = 0;
    this->state = ALPAR_STATE_HEADER_0;
    this->command = ALPAR_COMMAND_INVALID;
    this->buffer_idx = 0;
    memset(this->buffer, 0, sizeof(this->buffer));
    this->lrc = 0;
  }
};

SmartCard smartcard = SmartCard();
ALPAR alpar = ALPAR(smartcard);

void setup() {
  Serial.begin(115200);
}

void loop() {
  int chr = Serial.read();
  if (chr < 0) {
    return;
  }

  alpar.process(chr);
}