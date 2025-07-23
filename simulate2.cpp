#include <bits/stdc++.h>
typedef long long ll;
using std::array, std::vector, std::map, std::queue;
using std::cout, std::endl;

uint32_t xor128() {
    static uint32_t x = 123456789, y = 362436069, z = 521288629, w = 88675123;
    uint32_t t = x ^ (x << 11);
    x = y; y = z; z = w;
    return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
}
double nextDouble() {
    return double(xor128()) / 4294967296;
}

enum Group {
    EMERALD,
    TOPAZ,
    RUBY,
    SAPPHIRE,
    DIAMOND
};

int PLAYER_CNT;
vector<int> ratings;
vector<int> group_id;
vector<Group> group;
vector<int> wins;
vector<int> losses;
vector<int> total_wins;
vector<int> total_losses;
vector<queue<bool>> prev20; // 10 for emerald only
int group_cnt[5];
vector<map<int, int>> group_id_to_player;
vector<array<int, 5>> games_in_group;

// based on elo
constexpr auto expectation(int a, int b) -> double {
    double expected = 1 / (std::pow(10.0, (b - a) / 400.0) + 1);
    return expected;
}

bool flip(int p1, int p2) {
    return nextDouble() < expectation(ratings[p1], ratings[p2]);
}

void updateGroup(int player) {
    bool promote = false;
    bool demote = false;
    switch (group[player]) {
    case EMERALD: {
        // 5 out of 10
        promote = wins[player] >= 5;
        break;
    }
    case TOPAZ: {
        promote = wins[player] >= 11;
        demote = losses[player] >= 14;
        break;
    }
    case RUBY: {
        promote = wins[player] >= 12;
        demote = losses[player] >= 13;
        break;
    }
    case SAPPHIRE: {
        promote = wins[player] >= 13;
        demote = losses[player] >= 12;
        break;
    }
    case DIAMOND: {
        demote = losses[player] >= 11;
        break;
    }
    }
    if (promote || demote) {
        wins[player] = 0;
        losses[player] = 0;
        prev20[player] = {};
        int back_player = group_id_to_player[group[player]][group_cnt[group[player]] - 1];
        group_id[back_player] = group_id[player];
        group_id_to_player[group[player]][group_id[player]] = back_player;
        group_cnt[group[player]]--;
        if (promote) {
            group[player] = Group(group[player] + 1);
        } else {
            group[player] = Group(group[player] - 1);
        }
        group_cnt[group[player]]++;
        group_id[player] = group_cnt[group[player]] - 1;
        group_id_to_player[group[player]][group_cnt[group[player]] - 1] = player;
    }
}

void addWin(int player) {
    total_wins[player]++;
    wins[player]++;
    prev20[player].push(true);
    if (prev20[player].size() > 20
        || (group[player] == EMERALD && prev20[player].size() > 10)) {
        if (prev20[player].front()) {
            wins[player]--;
        } else {
            losses[player]--;
        }
        prev20[player].pop();
    }
}

void addLoss(int player) {
    total_losses[player]++;
    losses[player]++;
    prev20[player].push(false);
    if (prev20[player].size() > 20
        || (group[player] == EMERALD && prev20[player].size() > 10)) {
        if (prev20[player].front()) {
            wins[player]--;
        } else {
            losses[player]--;
        }
        prev20[player].pop();
    }
}

void runBattle(int p1, int p2) {
    games_in_group[p1][group[p1]]++;
    games_in_group[p2][group[p2]]++;
    if (flip(p1, p2)) {
        addWin(p1);
        addLoss(p2);
    } else {
        addWin(p2);
        addLoss(p1);
    }
    updateGroup(p1);
    updateGroup(p2);
}

void runTest(int gamecnt) {
    for (int i = 0; i < gamecnt; i++) {
        int p1 = xor128() % PLAYER_CNT;
        int p2 = -1;
        // if (group_cnt[group[p1]] < 2) {
        //     i--;
        //     continue;
        // }
        // int opp_group_id = xor128() % group_cnt[group[p1]];
        // p2 = group_id_to_player[group[p1]][opp_group_id];
        // if (p1 != p2) {
        //     runBattle(p1, p2);
        // }
        for (int reroll = 0; reroll < 5; reroll++) {
            if (nextDouble() < 0.8) {
                if (group_cnt[group[p1]] < 2) {
                    continue;
                }
                int opp_group_id = xor128() % group_cnt[group[p1]];
                p2 = group_id_to_player[group[p1]][opp_group_id];
            } else {
                if (group[p1] == EMERALD
                    || group_cnt[group[p1] - 1] == 0) {
                    continue;
                }
                int opp_group_id = xor128() % group_cnt[group[p1] - 1];
                p2 = group_id_to_player[group[p1] - 1][opp_group_id];
            }
            if (p2 != -1 && p2 != p1) {
                runBattle(p1, p2);
                break;
            }
        }
    }
}

int main() {
    std::ifstream fin("ratings.txt");
    fin >> PLAYER_CNT;
    ratings.assign(PLAYER_CNT, 0);
    for (auto& r : ratings) {
        fin >> r;
    }
    group_id.assign(PLAYER_CNT, 0);
    group.assign(PLAYER_CNT, EMERALD);
    group_id_to_player.assign(5, {});
    wins.assign(PLAYER_CNT, 0);
    losses.assign(PLAYER_CNT, 0);
    prev20.assign(PLAYER_CNT, {});
    games_in_group.assign(PLAYER_CNT, {});
    total_wins.assign(PLAYER_CNT, 0);
    total_losses.assign(PLAYER_CNT, 0);

    for (int player = 0; player < PLAYER_CNT; player++) {
        group_id[player] = player;
        group[player] = EMERALD;
        group_id_to_player[EMERALD][player] = player;
        group_cnt[EMERALD]++;
    }

    runTest(1000000);

    cout << std::fixed << std::setprecision(2);
    for (int i = 0; i < 5; i++) {
        cout << group_cnt[i] << " " << double(group_cnt[i]) / PLAYER_CNT << endl;
    }
    cout << endl;

    int cnt = PLAYER_CNT / 100;
    for (int i = 0; i < 100; i++) {
        int player = cnt * i;
        cout << double(player) / PLAYER_CNT << ": " << ratings[player] << endl;
        for (int g = 0; g < 5; g++) {
            cout << games_in_group[player][g] << " ";
        }
        cout << endl;
        cout << total_wins[player] << " " << total_losses[player] << endl;
    }
}
