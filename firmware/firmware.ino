#include <stdint.h>
#include <string.h>
#include <mbedtls/md.h>

typedef enum parser_state {
  STATE_READ_HEADER_0,
  STATE_READ_HEADER_1,
  STATE_READ_HEADER_2,
  STATE_READ_HEADER_3,
  STATE_READ_BODY,
  STATE_READ_LRC,
} parser_state_t;

typedef enum hsm_command {
  COMMAND_DIGEST_INIT_SHA1,
  COMMAND_DIGEST_INIT_SHA224,
  COMMAND_DIGEST_INIT_SHA256,
  COMMAND_DIGEST_INIT_SHA384,
  COMMAND_DIGEST_INIT_SHA512,
  COMMAND_DIGEST_INIT_RIPEMD160,
  COMMAND_DIGEST,
  COMMAND_DIGEST_UPDATE,
  COMMAND_DIGEST_KEY,
  COMMAND_DIGEST_FINAL,
} hsm_command_t;

typedef enum digest_algorithm {
  DIGEST_ALGORITHM_NONE,
  DIGEST_ALGORITHM_SHA1,
  DIGEST_ALGORITHM_SHA224,
  DIGEST_ALGORITHM_SHA256,
  DIGEST_ALGORITHM_SHA384,
  DIGEST_ALGORITHM_SHA512,
  DIGEST_ALGORITHM_RIPEMD160,
} digest_algorithm_t;

typedef struct digest {
  digest_algorithm_t algorithm;
  mbedtls_md_context_t ctx;
  uint8_t digest[64];
} digest_t;

typedef struct parser {
  parser_state_t state;
  hsm_command_t command;
  size_t remaining;
  int lrc;
} parser_t;

typedef struct {
  size_t length;
  uint8_t value[65536];
} buffer_t;

typedef struct {
  parser_t parser;
  buffer_t buffer;
  digest_t digest;
} hsm_t;

static hsm_t hsm;

static void clear_buffer() {
  hsm.buffer.length = 0;
  memset(hsm.buffer.value, 0, sizeof(hsm.buffer.value));
}

static void clear_mechanism_md() {
  hsm.digest.algorithm = DIGEST_ALGORITHM_NONE;
  mbedtls_md_init(&hsm.digest.ctx);
  memset(&hsm.digest.digest, 0, sizeof(hsm.digest.digest));
}

static void clear_mechanisms() {
  clear_mechanism_md();
}

static void clear_parser() {
  hsm.parser.remaining = 0;
  hsm.parser.state = STATE_READ_HEADER_0;
  hsm.parser.lrc = 0;
}

static void clear_all() {

  clear_buffer();
  clear_mechanisms();
}

static void digest_init(digest_algorithm_t algorithm) {
  const mbedtls_md_info_t *info;

  switch (algorithm) {
    case DIGEST_ALGORITHM_SHA1:
      info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);
      break;

    case DIGEST_ALGORITHM_SHA224:
      info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA224);
      break;

    case DIGEST_ALGORITHM_SHA256:
      info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
      break;

    case DIGEST_ALGORITHM_SHA384:
      info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA384);
      break;

    case DIGEST_ALGORITHM_SHA512:
      info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA512);
      break;

    case DIGEST_ALGORITHM_RIPEMD160:
      info = mbedtls_md_info_from_type(MBEDTLS_MD_RIPEMD160);
      break;

    default:
      return;
  }

  mbedtls_md_free(&hsm.digest.ctx);
  mbedtls_md_setup(&hsm.digest.ctx, info, 0);
  mbedtls_md_starts(&hsm.digest.ctx);

  hsm.digest.algorithm = algorithm;
  memset(&hsm.digest.digest, 0, sizeof(hsm.digest.digest));
}

static void process() {
  switch (hsm.parser.command) {
    case COMMAND_DIGEST_INIT_SHA1:
      digest_init(DIGEST_ALGORITHM_SHA1);
      break;

    case COMMAND_DIGEST_INIT_SHA224:
      digest_init(DIGEST_ALGORITHM_SHA224);
      break;

    case COMMAND_DIGEST_INIT_SHA256:
      digest_init(DIGEST_ALGORITHM_SHA256);
      break;

    case COMMAND_DIGEST_INIT_SHA384:
      digest_init(DIGEST_ALGORITHM_SHA384);
      break;

    case COMMAND_DIGEST_INIT_SHA512:
      digest_init(DIGEST_ALGORITHM_SHA512);
      break;

    case COMMAND_DIGEST_INIT_RIPEMD160:
      digest_init(DIGEST_ALGORITHM_RIPEMD160);
      break;

    case COMMAND_DIGEST:
      if (mbedtls_md_update(&hsm.digest.ctx, hsm.buffer.value, hsm.buffer.length)) {
      }

      if (mbedtls_md_finish(&hsm.digest.ctx, hsm.digest.digest)) {
      }
      break;

    case COMMAND_DIGEST_UPDATE:
      if (mbedtls_md_update(&hsm.digest.ctx, hsm.buffer.value, hsm.buffer.length)) {
      }
      break;

    case COMMAND_DIGEST_FINAL:
      if (mbedtls_md_finish(&hsm.digest.ctx, hsm.digest.digest)) {
      }
      break;
    case COMMAND_DIGEST_KEY:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  clear_all();
}

void loop() {
  int chr = Serial.read();
  if (chr < 0) {
    return;
  }

  switch (hsm.parser.state) {
    // read magic word
    case STATE_READ_HEADER_0:
      if (chr == 0x60) {
        hsm.parser.state = STATE_READ_HEADER_1;
        hsm.parser.lrc = (chr & 0xff);
      } else {
        clear_all();
      }
      break;

      // read length MSB
    case STATE_READ_HEADER_1:
      hsm.parser.remaining = (chr & 0xff) << 8;
      hsm.parser.state = STATE_READ_HEADER_2;
      hsm.parser.lrc ^= (chr & 0xff);
      break;

      // read length LSB
    case STATE_READ_HEADER_2:
      hsm.parser.remaining |= (chr & 0xff);
      hsm.parser.state = STATE_READ_HEADER_3;
      hsm.parser.lrc ^= (chr & 0xff);
      break;

      // read command
    case STATE_READ_HEADER_3:
      hsm.parser.command = (hsm_command_t)(chr & 0xff);
      hsm.parser.state = STATE_READ_BODY;
      hsm.parser.lrc ^= (chr & 0xff);
      clear_buffer();
      break;

      // read body
    case STATE_READ_BODY:
      if (hsm.parser.remaining > 0) {
        hsm.buffer.value[hsm.buffer.length] = chr & 0xff;
        hsm.parser.lrc ^= (chr & 0xff);
        hsm.buffer.length++;
        hsm.parser.remaining--;
      }

      hsm.parser.state = hsm.parser.remaining > 0 ? STATE_READ_BODY : STATE_READ_LRC;
      break;

      // read and compare LRC
    case STATE_READ_LRC:
      hsm.parser.lrc ^= (chr & 0xff);
      if (hsm.parser.lrc == 0x00) {
        process();
      } else {
        clear_all();
      }
      break;
  }
}
