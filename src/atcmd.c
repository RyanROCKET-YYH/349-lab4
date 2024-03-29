/**
 * @file atcmd.c
 *
 * @brief some functions about AT command
 *
 * @date 03/29/2024
 *
 * @author Yuhong Yao (yuhongy), Yiying Li (yiyingl4)
 */

#include <atcmd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>


/** @brief define unused variables */
#define UNUSED __attribute__((unused))

/** @brief define a variable to command mode */
int isInCommandMode = 1;

/**
 * @brief  initialize the atcmd_parser
 *
*/
void atcmd_parser_init(UNUSED atcmd_parser_t *parser, UNUSED const atcmd_t *atcmds, UNUSED uint32_t num_atcmds) {
    memset(parser, 0x00, sizeof(atcmd_parser_t));
    parser->atcmds = atcmds;
    parser->num_atcmds = num_atcmds;
}

/**
 * @brief detect +++ command
 *
*/
uint8_t atcmd_detect_escape(UNUSED atcmd_parser_t *parser, UNUSED char c) {
    static uint8_t plusCount = 0; // Count of '+'

    if (c == '+') {
        plusCount++;
        if (plusCount == 3) {
            // Detected three consecutive '+'
            plusCount = 0; // Reset count
            return 1; // Indicate that escape sequence has been detected.
        }
    } else {
        plusCount = 0; // Reset count if any other character is received.
    }
    return 0; // Escape sequence not detected or incomplete.
}

/**
 * @brief check each command stored in the atcmd parser to see if its cmdstr matches the provided command name
 *        return 1 if command is found; return 0 when command is not found
*/
uint8_t atcmd_parse(UNUSED atcmd_parser_t *parser, UNUSED char *cmd) {

    if (cmd == NULL || parser == NULL) {
        printf("Command or Parser is NULL.\n");
    }

    // command doesn't start with AT+
    if (strncmp(cmd, "AT+", 3) != 0) {
        printf("Command does not start with 'AT+'.\n");
        return 0; // Command is not valid
    }

    cmd += 3; // Skip "AT+" prefix to get to the command part
    char *equalSign = strchr(cmd, '=');
    size_t cmdNameLen = equalSign ? (size_t)(equalSign - cmd) : strlen(cmd);
    char *cmdArgs = equalSign ? equalSign + 1 : NULL;

    for (uint32_t i = 0; i < parser->num_atcmds; i++) {
        if (strncmp(parser->atcmds[i].cmdstr, cmd, cmdNameLen) == 0 && parser->atcmds[i].cmdstr[cmdNameLen] == '\0') {
            // A matching command has been found
            if (parser->atcmds[i].fn(parser->atcmds[i].args, cmdArgs)) {
                // printf("Command '%s' executed successfully.\n", parser->atcmds[i].cmdstr);
                return 1; // Command executed successfully
            } else {
                printf("Command '%s' failed to execute.\n", parser->atcmds[i].cmdstr);
                return 0; // Command execution failed
            }
        }
    }

    // If no command matches
    printf("Invalid or unrecognized command.\n");
    return 0; // Command is not valid
}
