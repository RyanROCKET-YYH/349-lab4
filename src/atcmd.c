/**
 * @file i2c.c
 *
 * @brief
 *
 * @date
 *
 * @author
 */

#include <atcmd.h>
#include <string.h>

#define UNUSED __attribute__((unused))

void atcmd_parser_init(UNUSED atcmd_parser_t *parser, UNUSED const atcmd_t *atcmds, UNUSED uint32_t num_atcmds) {
    memset(parser, 0x00, sizeof(atcmd_parser_t));
    parser->atcmds = atcmds;
    parser->num_atcmds = num_atcmds;
}

uint8_t atcmd_detect_escape(UNUSED atcmd_parser_t *parser, UNUSED char c) {
    static uint8_t escape_seq_detected = 0;
    if (c == '+') {
        escape_seq_detected = !escape_seq_detected;
        if (escape_seq_detected) {
            printf("Command mode entered.\n");
        } else {
            printf("Command mode exited.\n");
        }
        return 1;
    }
    return 0;
}

// check each command stored in the atcmd parser to see if its cmdstr matches the provided command name
// return 1 if command is found; return 0 when command is not found
uint8_t atcmd_parse(UNUSED atcmd_parser_t *parser, UNUSED char *cmd) {
    // command doesn't start with AT+
    if (strncmp(cmd, "AT+", 3) != 0) {
        printf("Command does not start with 'AT+'.\n");
        return 0;
    }

    cmd += 3; // the part after "AT+"

    // check if "AT+RESUME"
    if (strcmp(cmd, "RESUME") == 0) {
        printf("AT+RESUME command executed successfully.\n");
        return 1;
    }
    // check if "AT+HELLO=<name>"
    else if (strncmp(cmd, "HELLO=", 6) == 0) {
        char *name = cmd + 6;
        printf("HELLO command executed with name: %s\n", name);
        return 1;
    }
    // check if "AT+PASSCODE?"
    else if (strcmp(cmd, "PASSCODE?") == 0) {
        printf("AT+PASSCODE? executed successfully.\n");
        return 1;
    }
    // check if "AT+PASSCODE=<passcode>"
    else if (strncmp(cmd, "PASSCODE=", 9) == 0) {
        char *passcode = cmd + 9;
        printf("PASSCODE command executed with passcode: %s\n", passcode);
        return 1;
    }

    // if it doesn't match any of the defined command
    printf("Invalid or unrecognized command.\n");
    return 0;
}
