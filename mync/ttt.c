#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

char board[9];
char strategy[9];

void initializeBoard() {
    for (int i = 0; i < 9; ++i) {
        board[i] = ' ';
    }
}

void printBoard() {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            printf("%c", board[i * 3 + j]);
            if (j < 2) printf(" | ");
        }
        printf("\n");
        if (i < 2) printf("--|---|--\n");
    }
    printf("\n");
    fflush(stdout);
}

bool playerMove(int index, char player) {
    index -= 1; // Convert to 0-based index
    if (board[index] == ' ') {
        board[index] = player;
        return true;
    } else {
        return false;
    }
}

bool checkWin(char player) {
    // Check rows and columns
    for (int i = 0; i < 3; ++i) {
        if (board[i * 3] == player && board[i * 3 + 1] == player && board[i * 3 + 2] == player)
            return true;
        if (board[i] == player && board[i + 3] == player && board[i + 6] == player)
            return true;
    }
    // Check diagonals
    if (board[0] == player && board[4] == player && board[8] == player)
        return true;
    if (board[2] == player && board[4] == player && board[6] == player)
        return true;
    return false;
}

bool isBoardFull() {
    for (int i = 0; i < 9; ++i) {
        if (board[i] == ' ') {
            return false;
        }
    }
    return true;
}

bool GameEnd() {
    if (checkWin('X')) {
        printf("I win!\n");
        return true;
    } else if (checkWin('O')) {
        printf("I lost!\n");
        return true;
    } else if (isBoardFull()) {
        printf("DRAW\n");
        return true;
    }
    return false;
}

void programMove() {
    for (int i = 0; i < 9; ++i) {
        int move = strategy[i] - '0';
        if (playerMove(move, 'O')) {
            printf("%d\n", move);
            fflush(stdout);
            return;
        }
    }
}

bool validateStrategy(const char* str) {
    if (strlen(str) != 9) return false;
    int count[10] = {0};
    for (int i = 0; i < 9; ++i) {
        int digit = str[i] - '0';
        if (digit < 1 || digit > 9) return false;
        if (count[digit]++ > 0) return false;
    }
    return true;
}

void play() {
    char player_input[3];

    programMove();
    printBoard();

    while (1) {
        printf("Enter your move (1-9): ");
        fflush(stdout);
        if (fgets(player_input, sizeof(player_input), stdin) != NULL) {
            int player_move = atoi(player_input);
            if (player_move >= 1 && player_move <= 9) {
                if (playerMove(player_move, 'X')) {
                    printBoard();
                } else {
                    printf("Invalid move. Try again.\n");
                    continue;
                }
            } else {
                printf("Invalid input. Enter a number between 1 and 9.\n");
                continue;
            }
        }
        fflush(stdout);

        if (GameEnd()) break;
        fflush(stdout);

        programMove();
        printBoard();
        if (GameEnd()) break;
        fflush(stdout);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2 || !validateStrategy(argv[1])) {
        printf("Error\n");
        fflush(stdout);
        return 1;
    }

    strcpy(strategy, argv[1]);
    initializeBoard();
    play();
    return 0;
}
