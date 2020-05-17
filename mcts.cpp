#include <algorithm>
#include <cmath>
#include <cstdio>
#include <random>
#include <vector>

using namespace std;

//#define DEBUG

const int BOARD_SIZE = 9;
const int LINE_SIZE = 5;
typedef __uint128_t ptype;

const double eps = 1e-6;

mt19937 gen;

inline ptype ptype_pow2(int x) {
	ptype r = 1;
	return r << x;
}

void ptype_print(ptype x) {
	for (int i=0; i<BOARD_SIZE * BOARD_SIZE; ++i) {
		putchar("10"[!(x & ptype_pow2(i))]);
	}
	putchar('\n');
}

int ptype_pick(const ptype &x) {
	unsigned long long u = (unsigned long long)(x >> 64), v = (unsigned long long)x;
	int ub = __builtin_popcountll(u), vb = __builtin_popcountll(v);
	uniform_int_distribution<> dist(0, ub + vb - 1);
	int idx = dist(gen);
	unsigned long long target = idx >= ub ? v : u;
	unsigned long long result = idx >= ub ? 0 : 64;
	if (idx >= ub) idx -= ub;
	while (idx--) target &= target - 1; // TODO: optimize!
	return result | __builtin_ctzll(target);
}

const ptype FULL_BOARD = ptype_pow2(BOARD_SIZE * BOARD_SIZE) - 1;
class board {
private:
	int result_status;
	pair<ptype, ptype> data;
public:
	board() {
		this->reset();
	}
	board(const board &another) {
		this->result_status = another.result_status;
		this->data.first = another.data.first;
		this->data.second = another.data.second;
	}
	void reset() {
		result_status = 0;
		data.first = data.second = 0;
	}
	bool action(int pos, bool last) {
		if (result_status < 0) return true;
		ptype flag = ptype_pow2(pos);
		if (!(data.first & flag) && !(data.second & flag)) {
			ptype &target = last ? data.second : data.first;
			target |= flag;
			if (++result_status == BOARD_SIZE * BOARD_SIZE) {
				result_status = -3;
				return true;
			}
			for (int k=0; k<4; ++k) {
				int c = -1;
				int x = pos % BOARD_SIZE, y = pos / BOARD_SIZE;
				for (int u=0; u<BOARD_SIZE; ++u) {
					if (target & ptype_pow2(y * 9 + x)) ++c;
					else break;
					x += k%3 - 1; y += k/3 - 1;
					x = x < 0 ? x + BOARD_SIZE : x >= BOARD_SIZE ? x - BOARD_SIZE : x;
					y = y < 0 ? y + BOARD_SIZE : y >= BOARD_SIZE ? y - BOARD_SIZE : y;
				}
				x = pos % BOARD_SIZE; y = pos / BOARD_SIZE;
				for (int u=0; u<BOARD_SIZE; ++u) {
					if (target & ptype_pow2(y * 9 + x)) ++c;
					else break;
					x -= k%3 - 1; y -= k/3 - 1;
					x = x < 0 ? x + BOARD_SIZE : x >= BOARD_SIZE ? x - BOARD_SIZE : x;
					y = y < 0 ? y + BOARD_SIZE : y >= BOARD_SIZE ? y - BOARD_SIZE : y;
				}
				if (c >= LINE_SIZE) {
					result_status = last ? -2 : -1;
					return true;
				}
			}
		}
		return false;
	}
	bool is_finished() const {
		return result_status < 0;
	}
	int winner() const {
		return result_status == -3 ? 0 : result_status >= 0 ? -1 : -result_status;
	}
	ptype possible_position() const {
		return FULL_BOARD & ~(data.first | data.second);
	}
};

struct tree_entry {
	int win, total;
	ptype next_candidate;
	tree_entry *array[BOARD_SIZE * BOARD_SIZE];
	double max_value;
};

class tree_manager {
public:
	tree_entry *tree;
	int total_node_count, whole_size;
	tree_manager(int whole_size) {
		this->tree = (tree_entry *)malloc(sizeof(tree_entry) * whole_size);
		this->total_node_count = 0;
		this->whole_size = whole_size;
	}
	tree_entry *new_node() {
		if (this->total_node_count >= this->whole_size) {
			return nullptr;
		}
		tree_entry *result = this->tree + this->total_node_count++;
		result->win = result->total = 0;
		result->next_candidate = 0;
		memset(result->array, 0, sizeof(tree_entry *) * BOARD_SIZE * BOARD_SIZE);
		result->max_value = 0;
		return result;
	}
	~tree_manager() {
		free(this->tree);
	}
};

double get_value(double this_win, double this_total, double parent_total) {
	if (abs(this_total) < eps) return 1e6;
	return this_win / this_total + sqrt(2 * log(parent_total) / this_total);
}

int mcts(const board &now, const int mc_count = 10000, const int repeat_count = 10) {
	const ptype possible_pos = now.possible_position();
	tree_manager manager(mc_count + 5);
	tree_entry *const tree = manager.new_node();
	tree->max_value = 1e6;
	tree->next_candidate = possible_pos;
	for (int mc_epoch = 1; mc_epoch <= mc_count; ++mc_epoch) {
		board mc_board(now);
		tree_entry *tree_ptr = tree;
		double parent_total = (mc_epoch - 1) * repeat_count;
		bool opponent_turn = false;
		ptype possible = possible_pos;
		vector<int> backprop;
		// selection
		while (!mc_board.is_finished()) {
			int idx;
			if (!tree_ptr->next_candidate) {
				double &mx = tree_ptr->max_value;
				mx = -1;
				for (int i=0; i<BOARD_SIZE * BOARD_SIZE; ++i) {
					tree_entry *ptr = tree_ptr->array[i];
					if (ptr) mx = max(mx, get_value(ptr->win, ptr->total, parent_total));
				}
				for (int i=0; i<BOARD_SIZE * BOARD_SIZE; ++i) {
					tree_entry *ptr = tree_ptr->array[i];
					if (ptr) {
						double value = get_value(ptr->win, ptr->total, parent_total);
						if (abs(value - mx) < eps) {
							tree_ptr->next_candidate |= ptype_pow2(i);
						}
					}
				}
			}
			idx = ptype_pick(tree_ptr->next_candidate);
			backprop.push_back(idx);
			mc_board.action(idx, opponent_turn);
			possible &= ~ptype_pow2(idx);
			if (!tree_ptr->array[idx]) {
				tree_ptr->array[idx] = manager.new_node();
				tree_ptr->array[idx]->next_candidate = possible;
				tree_ptr->array[idx]->max_value = 1e6;
				break;
			}
			parent_total = tree_ptr->total;
			tree_ptr = tree_ptr->array[idx];
			opponent_turn = !opponent_turn;
		}
#ifdef DEBUG
		for (const int &bp: backprop) {
			printf("%6d ", bp);
		}
		puts("");
#endif
		// playout
		int win_count = 0, lose_count = 0;
		if (!mc_board.is_finished()) {
			for (int k=1; k<=repeat_count; ++k) {
				ptype pos = possible;
				board playout(mc_board);
				bool turn = opponent_turn;
				while (!playout.is_finished()) {
					int idx = ptype_pick(pos);
					playout.action(idx, turn);
					pos &= ~ptype_pow2(idx);
					turn = !turn;
				}
				const int winner = playout.winner();
				if (winner == 1) ++win_count;
				else if (winner == 2) ++lose_count;
			}
		} else {
			int winner = mc_board.winner();
			win_count = winner == 1 ? repeat_count : 0;
			lose_count = winner == 2 ? repeat_count : 0;
		}
		// backpropagation
		tree_ptr = tree;
		opponent_turn = true;
		tree->win += win_count;
		tree->total += repeat_count;
		for (const int &idx : backprop) {
			auto ptr = tree_ptr->array[idx];
			ptr->win += opponent_turn ? win_count : lose_count;
			ptr->total += repeat_count;
#ifdef DEBUG
			printf("%.4lf ", ptr->win / (double)ptr->total);
#endif
			const double value = get_value(ptr->win, ptr->total, tree_ptr->total);
			double &object = tree_ptr->max_value;
			if (abs(value - object) < eps) {
				tree_ptr->next_candidate |= ptype_pow2(idx);
			} else if (value < object) {
				tree_ptr->next_candidate &= ~ptype_pow2(idx);
			} else {
				object = value;
				tree_ptr->next_candidate = ptype_pow2(idx);
			}
			tree_ptr = ptr;
			opponent_turn = !opponent_turn;
		}
#ifdef DEBUG
		puts("");
#endif
	}
	int mxidx = -1;
	/* last selection would be without the exploration factor */ {
		double mx = -1;
		int now_idx;
		for (ptype pos = possible_pos; pos; pos &= ~ptype_pow2(now_idx)) {
			now_idx = ptype_pick(pos);
			tree_entry *node = tree->array[now_idx];
			if (node) {
				double score = node->total ? node->win / (double)node->total : 0;
				if (mx + eps < score) {
					mx = score;
					mxidx = now_idx;
				}
			}
		}
	}
	return mxidx;
}

int main() {
	while (1) {
		board s;
		try {
			for (int i=0; i<81; ++i) {
				char c;
				do {
					if (scanf("%c", &c) < 1) {
						throw -1;
					}
				} while (c != '0' && c != '1' && c != '2');
				if (c != '0') s.action(i, c == '2');
			}
		} catch (int x) {
			if (x == -1) break;
		}
		int answer = s.is_finished() ? -1 : mcts(s, 100000);
		if (answer >= 0) s.action(answer, false);
		printf("%d %d\n", answer, s.winner());
		fflush(stdout);
	}
	return 0;
}
