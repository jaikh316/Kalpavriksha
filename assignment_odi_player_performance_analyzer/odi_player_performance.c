#define Player HeaderPlayer
#include "Players_data.h"
#undef Player

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <math.h>

#define MAX_NAME_LENGTH 50
#define TEAM_COUNT 10
#define MAX_PLAYERS_PER_TEAM 50
#define NO_LIMIT 1000000000
#define LINE "-------------------------------------------------------------------------------------------------------------\n"

typedef enum { BATSMAN = 1, BOWLER = 2, ALL_ROUNDER = 3 } Role;

typedef struct {
    int playerId;
    char name[MAX_NAME_LENGTH + 1];
    char teamName[MAX_NAME_LENGTH + 1];
    Role role;
    int totalRuns;
    float battingAverage;
    float strikeRate;
    int wickets;
    float economyRate;
    float performanceIndex;
} Player;

typedef struct PlayerNode {
    Player data;
    struct PlayerNode *next;
} PlayerNode;

typedef struct SortedNode {
    PlayerNode *player;
    struct SortedNode *next;
} SortedNode;

typedef struct {
    int teamId;
    char name[MAX_NAME_LENGTH + 1];
    int totalPlayers;
    float averageBattingStrikeRate;

    PlayerNode *players;
    PlayerNode *playersTail;
    SortedNode *roleHeads[4];
} Team;


typedef struct OutputNode {
    Player *player;
    struct OutputNode *next;
} OutputNode;

static Team teamList[TEAM_COUNT];

const char *roleName(Role role) {
    switch (role) {
        case BATSMAN: return "Batsman";
        case BOWLER: return "Bowler";
        case ALL_ROUNDER: return "All-rounder";
        default: return "Unknown";
    }
}

void clearInputLine(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        /* Discard remaining characters. */
    }
}

int readInt(const char *prompt, int *value, int min, int max) {
    char input[128];
    char *endPointer;
    long parsedValue;

    printf("%s", prompt);

    if (fgets(input, sizeof(input), stdin) == NULL) {
        return 0;
    }

    errno = 0;
    parsedValue = strtol(input, &endPointer, 10);
    if (endPointer == input || errno == ERANGE || parsedValue < INT_MIN ||
        parsedValue > INT_MAX) {
        printf("Invalid input. Please enter a valid integer.\n");
        return 0;
    }

    while (*endPointer == ' ' || *endPointer == '\t' || *endPointer == '\r' || *endPointer == '\n') {
        endPointer++;
    }

    if (*endPointer != '\0') {
        printf("Invalid input. Please enter only an integer.\n");
        return 0;
    }

    if (parsedValue < min || parsedValue > max) {
        printf("Invalid value. Please enter a value between %d and %d.\n", min, max);
        return 0;
    }

    *value = (int)parsedValue;
    return 1;
}

int readFloat(const char *prompt, float *value) {
    char input[128];
    char *endPointer;
    float parsedValue;

    printf("%s", prompt);
    if (fgets(input, sizeof(input), stdin) == NULL) {
        return 0;
    }

    errno = 0;
    parsedValue = strtof(input, &endPointer);
    if(endPointer == input || errno == ERANGE || !isfinite(parsedValue) || parsedValue < 0.0f) {
        printf("Invalid input. Please enter a finite, non-negative number.\n");
        return 0;
    }

    while(*endPointer == ' ' || *endPointer == '\t' || *endPointer == '\r' || *endPointer == '\n') {
        endPointer++;
    }

    if(*endPointer != '\0') {
        printf("Invalid input. Please enter only a number.\n");
        return 0;
    }

    *value = parsedValue;
    return 1;
}

Role readRole(void) {
    int choice;

    while (!readInt("Role (1-Batsman, 2-Bowler, 3-All-rounder): ", &choice, BATSMAN, ALL_ROUNDER)) {
    }
    return (Role)choice;
}

void readLine(char *destination, int size) {
    if (fgets(destination, size, stdin) != NULL) {
        size_t length = strlen(destination);

        if (length > 0 && destination[length - 1] == '\n') {
            destination[length - 1] = '\0';
        } else {
            clearInputLine();
        }
    } else {
        destination[0] = '\0';
    }
}

float calculatePerformanceIndex(const Player *player) {
    float battingScore = (player->battingAverage * player->strikeRate) / 100.0f;

    if(player->role == BATSMAN) {
        return battingScore;
    }

    if(player->role == BOWLER) {
        return (player->wickets * 2.0f) + (100.0f - player->economyRate);
    }
    return battingScore + (player->wickets * 2.0f);
}


void updateTeamStatistics(Team *team) {
    PlayerNode *current = team->players;
    float strikeRateTotal = 0.0f;
    int battingPlayerCount = 0;
    int total = 0;

    while(current != NULL) {
        total++;

        if (current->data.role == BATSMAN || current->data.role == ALL_ROUNDER) {
            strikeRateTotal += current->data.strikeRate;
            battingPlayerCount++;
        }
        current = current->next;
    }
    team->totalPlayers = total;

    if (battingPlayerCount == 0) {
        team->averageBattingStrikeRate = 0.0f;
    } else {
        team->averageBattingStrikeRate = strikeRateTotal / battingPlayerCount;
    }
}


Team *searchTeamById(int teamId) {
    int low = 0, high = TEAM_COUNT - 1;

    while (low <= high) {
        int middle = low + (high - low) / 2;

        if (teamList[middle].teamId == teamId) {
            return &teamList[middle];
        }
        if (teamList[middle].teamId < teamId) {
            low = middle + 1;
        } else {
            high = middle - 1;
        }
    }
    return NULL;
}

Team *readTeam(void) {
    int teamId;
    Team *team;

    if (!readInt("Enter Team ID (1-10): ", &teamId, 1, TEAM_COUNT)) {
    return NULL;
    }

    team = searchTeamById(teamId);
    if (team == NULL) {
        printf("Team not found.\n");
    }
    return team;
}

int playerIdExists(int playerId) {
    int teamIndex;
    for(teamIndex = 0; teamIndex < TEAM_COUNT; teamIndex++) {
        PlayerNode *current = teamList[teamIndex].players;
        while(current != NULL) {
            if(current->data.playerId == playerId) {
                return 1;
            }
            current = current->next;
        }
    }
    return 0;
}


int insertPlayer(Team *team, const Player *player) {
    PlayerNode *newPlayer;
    SortedNode *newSortedNode;
    SortedNode **roleHead;
    SortedNode *current;

    newPlayer = (PlayerNode *)malloc(sizeof(PlayerNode));
    newSortedNode = (SortedNode *)malloc(sizeof(SortedNode));

    if (newPlayer == NULL || newSortedNode == NULL) {
        free(newPlayer);
        free(newSortedNode);
        printf("Memory allocation failed. Player was not added.\n");
        return 0;
    }

    newPlayer->data = *player;
    newPlayer->next = NULL;
    newSortedNode->player = newPlayer;
    newSortedNode->next = NULL;

    if (team->players == NULL) {
        team->players = newPlayer;
    } else {
        team->playersTail->next = newPlayer;
    }
    team->playersTail = newPlayer;

    roleHead = &team->roleHeads[player->role];
    current = *roleHead;

    if (current == NULL ||
        current->player->data.performanceIndex < player->performanceIndex) {
        newSortedNode->next = *roleHead;
        *roleHead = newSortedNode;
    } else {
        while (current->next != NULL &&
               current->next->player->data.performanceIndex >= player->performanceIndex) {
            current = current->next;
        }
        newSortedNode->next = current->next;
        current->next = newSortedNode;
    }
    updateTeamStatistics(team);
    return 1;
}


int initializeTeams(void) {
    int teamIndex;

    for (teamIndex = 0; teamIndex < TEAM_COUNT; teamIndex++) {
        teamList[teamIndex].teamId = teamIndex + 1;
        strcpy(teamList[teamIndex].name, teams[teamIndex]);
    }

    for (int playerIndex = 0; playerIndex < playerCount; playerIndex++) {
        Player newPlayer = {0};
        int matchingTeamIndex;

        newPlayer.playerId = players[playerIndex].id;
        strncpy(newPlayer.name, players[playerIndex].name, MAX_NAME_LENGTH);
        newPlayer.name[MAX_NAME_LENGTH] = '\0';
        strncpy(newPlayer.teamName, players[playerIndex].team, MAX_NAME_LENGTH);
        newPlayer.teamName[MAX_NAME_LENGTH] = '\0';
        newPlayer.totalRuns = players[playerIndex].totalRuns;
        newPlayer.battingAverage = players[playerIndex].battingAverage;
        newPlayer.strikeRate = players[playerIndex].strikeRate;
        newPlayer.wickets = players[playerIndex].wickets;
        newPlayer.economyRate = players[playerIndex].economyRate;

        if (strcmp(players[playerIndex].role, "Batsman") == 0) {
            newPlayer.role = BATSMAN;
        } else if (strcmp(players[playerIndex].role, "Bowler") == 0) {
            newPlayer.role = BOWLER;
        } else {
            newPlayer.role = ALL_ROUNDER;
        }

        newPlayer.performanceIndex = calculatePerformanceIndex(&newPlayer);

        for (matchingTeamIndex = 0; matchingTeamIndex < TEAM_COUNT; matchingTeamIndex++) {
            if (strcmp(teamList[matchingTeamIndex].name, players[playerIndex].team) == 0) {
                break;
            }
        }

        if (matchingTeamIndex == TEAM_COUNT) {
            continue;
        }

        if (teamList[matchingTeamIndex].totalPlayers >= MAX_PLAYERS_PER_TEAM) {
            printf("Team %s is full. Could not load player: %s\n", teamList[matchingTeamIndex].name, newPlayer.name);
        } else if (!insertPlayer(&teamList[matchingTeamIndex], &newPlayer)) {
            printf("Initialization stopped because memory allocation failed.\n");
            return 0;
        }
    }
    return 1;
}


void printPlayerHeader(int includeTeam) {
    printf(LINE);

    if (includeTeam) {
        printf("%-6s %-25s %-16s %-13s %-7s %-7s %-7s %-7s %-7s %-10s\n",
               "ID", "Name", "Team", "Role", "Runs", "Avg", "SR", "Wkts", "Econ", "Perf.Index");
    } else {
        printf("%-6s %-25s %-13s %-7s %-7s %-7s %-7s %-7s %-10s\n",
               "ID", "Name", "Role", "Runs", "Avg", "SR", "Wkts", "Econ", "Perf.Index");
    }

    printf(LINE);
}

void printPlayer(const Player *player, int includeTeam) {
    printf("%-6d %-25.25s ", player->playerId, player->name);

    if (includeTeam) {
        printf("%-16.16s ", player->teamName);
    }

    printf("%-13s %-7d %-7.1f %-7.1f %-7d %-7.1f %-10.2f\n",
           roleName(player->role), player->totalRuns, player->battingAverage, player->strikeRate, player->wickets, player->economyRate, player->performanceIndex);
}

void displayTeamPlayers(void) {
    Team *team = readTeam();
    PlayerNode *current;

    if (team == NULL) {
        return;
    }

    printf("\nPlayers of Team %s:\n", team->name);
    printPlayerHeader(0);
    current = team->players;
    if (current == NULL) {
        printf("No players found.\n");
    }

    while (current != NULL) {
        printPlayer(&current->data, 0);
        current = current->next;
    }
    printf(LINE);
    printf("Total Players: %d\n", team->totalPlayers);
    printf("Average Batting Strike Rate: %.2f\n", team->averageBattingStrikeRate);
}

void displayTeamsByStrikeRate(void) {
    Team *sortedTeams[TEAM_COUNT];
    int teamIndex;
    int candidateIndex;
    for (teamIndex = 0; teamIndex < TEAM_COUNT; teamIndex++) {
        sortedTeams[teamIndex] = &teamList[teamIndex];
    }

    for (teamIndex = 0; teamIndex < TEAM_COUNT - 1; teamIndex++) {
        int bestIndex = teamIndex;

        for (candidateIndex = teamIndex + 1; candidateIndex < TEAM_COUNT; candidateIndex++) {
            if (sortedTeams[candidateIndex]->averageBattingStrikeRate >
                sortedTeams[bestIndex]->averageBattingStrikeRate) {
                bestIndex = candidateIndex;
            }
        }

        if (bestIndex != teamIndex) {
            Team *temporary = sortedTeams[teamIndex];
            sortedTeams[teamIndex] = sortedTeams[bestIndex];
            sortedTeams[bestIndex] = temporary;
        }
    }

    printf("\nTeams Sorted by Average Batting Strike Rate\n");
    printf("---------------------------------------------------------------\n");
    printf("%-8s %-20s %-15s %-15s\n", "ID", "Team Name", "Avg Bat SR", "Total Players");
    printf("---------------------------------------------------------------\n");

    for (teamIndex = 0; teamIndex < TEAM_COUNT; teamIndex++) {
        printf("%-8d %-20s %-15.2f %-15d\n", sortedTeams[teamIndex]->teamId, sortedTeams[teamIndex]->name, sortedTeams[teamIndex]->averageBattingStrikeRate, sortedTeams[teamIndex]->totalPlayers);
    }
    printf("---------------------------------------------------------------\n");
}

void displayTopKPlayers(void) {
    Team *team = readTeam();
    Role role;
    int k;
    int displayed = 0;
    SortedNode *current;

    if (team == NULL) {
        return;
    }

    role = readRole();
    if (!readInt("Enter number of players (K): ", &k, 1, NO_LIMIT)) {
        return;
    }
    printf("\nTop %d %s(s) of Team %s:\n", k, roleName(role), team->name);
    printPlayerHeader(0);

    current = team->roleHeads[role];
    while (current != NULL && displayed < k) {
        printPlayer(&current->player->data, 0);
        current = current->next;
        displayed++;
    }

    if (displayed == 0) {
        printf("No players of this role found in the team.\n");
    }
    printf(LINE);
    printf("Players displayed: %d\n", displayed);
}

OutputNode *mergeTwoLists(OutputNode *first, OutputNode *second) {
    OutputNode dummy;
    OutputNode *tail = &dummy;
    dummy.next = NULL;

    while (first != NULL && second != NULL) {
        if (first->player->performanceIndex >= second->player->performanceIndex) {
            tail->next = first;
            first = first->next;
        } else {
            tail->next = second;
            second = second->next;
        }
        tail = tail->next;
    }
    tail->next = (first != NULL) ? first : second;
    return dummy.next;
}

OutputNode *mergeTeamLists(OutputNode *listArray[], int left, int right) {
    int middle;
    OutputNode *first;
    OutputNode *second;

    if (left == right) {
        return listArray[left];
    }

    middle = left + (right - left) / 2;
    first = mergeTeamLists(listArray, left, middle);
    second = mergeTeamLists(listArray, middle + 1, right);
    return mergeTwoLists(first, second);
}

void freeOutputList(OutputNode *head) {
    while (head != NULL) {
        OutputNode *temporary = head;
        head = head->next;
        free(temporary);
    }
}

int copyRoleList(SortedNode *source, OutputNode **copiedHead) {
    OutputNode *head = NULL;
    OutputNode *tail = NULL;
    *copiedHead = NULL;

    while (source != NULL) {
        OutputNode *newNode = (OutputNode *)malloc(sizeof(OutputNode));
        if (newNode == NULL) {
            freeOutputList(head);
            printf("Memory allocation failed while preparing the role ranking.\n");
            return 0;
        }

        newNode->player = &source->player->data;
        newNode->next = NULL;
        if (head == NULL) {
            head = newNode;
        } else {
            tail->next = newNode;
        }
        tail = newNode;
        source = source->next;
    }

    *copiedHead = head;
    return 1;
}

void displayAllPlayersOfRole(void) {
    Role role = readRole();
    OutputNode *teamLists[TEAM_COUNT];
    OutputNode *mergedList;
    OutputNode *current;
    int teamIndex;
    int total = 0;

    for(teamIndex = 0; teamIndex < TEAM_COUNT; teamIndex++) {
        teamLists[teamIndex] = NULL;
    }

    for(teamIndex = 0; teamIndex < TEAM_COUNT; teamIndex++) {
        if(!copyRoleList(teamList[teamIndex].roleHeads[role], &teamLists[teamIndex])) {
            int cleanupIndex;
            for(cleanupIndex = 0; cleanupIndex < TEAM_COUNT; cleanupIndex++) {
                freeOutputList(teamLists[cleanupIndex]);
            }
            printf("Could not display the ranking. Please try again.\n");
            return;
        }
    }

    mergedList = mergeTeamLists(teamLists, 0, TEAM_COUNT - 1);

    printf("\nAll %s Players Across All Teams\n", roleName(role));
    printPlayerHeader(1);

    current = mergedList;

    while(current != NULL) {
        printPlayer(current->player, 1);
        current = current->next;
        total++;
    }

    printf(LINE);
    printf("Total %s players: %d\n", roleName(role), total);
    freeOutputList(mergedList);
}


void addPlayer(void) {
    Team *team = readTeam();
    Player newPlayer;

    if (team == NULL) {
        return;
    }

    if(team->totalPlayers >= MAX_PLAYERS_PER_TEAM) {
        printf("This team has reached the maximum limit of 50 players.\n");
        return;
    }

    printf("\nEnter Player Details:\n");

    if(!readInt("Player ID (1-1000): ", &newPlayer.playerId, 1, 1000)) {
        return;
    }
    if(playerIdExists(newPlayer.playerId)) {
        printf("A player with this ID already exists.\n");
        return;
    }

    printf("Name: ");
    readLine(newPlayer.name, sizeof(newPlayer.name));

    if(strlen(newPlayer.name) == 0) {
        printf("Name cannot be empty.\n");
        return;
    }
    strcpy(newPlayer.teamName, team->name);
    newPlayer.role = readRole();

    if (!readInt("Total Runs: ", &newPlayer.totalRuns, 0, NO_LIMIT) ||
        !readFloat("Batting Average: ", &newPlayer.battingAverage) ||
        !readFloat("Strike Rate: ", &newPlayer.strikeRate) ||
        !readInt("Wickets: ", &newPlayer.wickets, 0, NO_LIMIT) ||
        !readFloat("Economy Rate: ", &newPlayer.economyRate)) {
        printf("Player was not added.\n");
        return;
    }

    newPlayer.performanceIndex = calculatePerformanceIndex(&newPlayer);

    if (insertPlayer(team, &newPlayer)) {
        printf("\nPlayer added successfully to Team %s!\n", team->name);
        printf("Calculated Performance Index: %.2f\n", newPlayer.performanceIndex);
    }
}


void freeAllMemory(void) {
    int teamIndex;
    int roleIndex;

    for (teamIndex = 0; teamIndex < TEAM_COUNT; teamIndex++) {
        PlayerNode *playerCurrent = teamList[teamIndex].players;

        while (playerCurrent != NULL) {
            PlayerNode *temporary = playerCurrent;
            playerCurrent = playerCurrent->next;
            free(temporary);
        }

        for (roleIndex = BATSMAN; roleIndex <= ALL_ROUNDER; roleIndex++) {
            SortedNode *sortedCurrent = teamList[teamIndex].roleHeads[roleIndex];

            while (sortedCurrent != NULL) {
                SortedNode *temporary = sortedCurrent;
                sortedCurrent = sortedCurrent->next;
                free(temporary);
            }
        }
    }
}


void displayMenu(void) {
    printf("\n============================================================\n"
           "             ICC ODI PLAYER PERFORMANCE ANALYZER\n"
           "============================================================\n"
           "1. Add Player to Team\n"
           "2. Display Players of a Specific Team\n"
           "3. Display Teams by Average Batting Strike Rate\n"
           "4. Display Top K Players of a Specific Team by Role\n"
           "5. Display All Players of a Specific Role Across All Teams\n"
           "6. Exit\n"
           "============================================================\n");
}

int main(void) {
    int choice = 0;
    (void)teamCount;
    if (!initializeTeams()) {
        freeAllMemory();
        printf("Initialization failed. The program will now exit.\n");
        return 1;
    }

    printf("ICC ODI Player Performance Analyzer initialized successfully.\n");
    printf("Loaded %d predefined players across %d teams.\n", playerCount, TEAM_COUNT);

    do {
        displayMenu();
        if (!readInt("Enter your choice: ", &choice, 1, 6)) {
            if (feof(stdin)) {
                printf("\nInput ended. Releasing memory and exiting.\n");
                break;
            }
            choice = 0;
            continue;
        }

        switch (choice) {
            case 1: addPlayer(); break;
            case 2: displayTeamPlayers(); break;
            case 3: displayTeamsByStrikeRate(); break;
            case 4: displayTopKPlayers(); break;
            case 5: displayAllPlayersOfRole(); break;
            case 6:
                printf("Exiting ICC ODI Player Performance Analyzer.\n");
                break;
        }
    } while (choice != 6);
    freeAllMemory();
    return 0;
}