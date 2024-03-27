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

uint8_t atcmd_parse(UNUSED atcmd_parser_t *parser, UNUSED char *cmd) {
    for (uint32_t i = 0; i < parser->num_atcmds; i++) {
        const atcmd_t *atcmd = &parser->atcmds[i];
        // check if it starts with "AT+"
        if (strncmp(cmd, "AT+", 3) == 0 && strcmp(cmd + 3, atcmd->cmdstr) == 0) {
            if (atcmd->fn(atcmd->args, NULL)) {
                printf("Command %s executed successfully.\n", atcmd->cmdstr);
                return 1;
            }
        }
    }
    printf("Command not found.\n");
    return 0;
}
