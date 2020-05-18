/*
* Pacman2 样例程序
* 作者：zhouhy
* 时间：2016/10/12 12:54
*
* 【命名惯例】
*  r/R/y/Y：Row，行，纵坐标
*  c/C/x/X：Column，列，横坐标
*  数组的下标都是[y][x]或[r][c]的顺序
*  玩家编号0123
*
* 【坐标系】
*   0 1 2 3 4 5 6 7 8
* 0 +----------------> x
* 1 |
* 2 |
* 3 |
* 4 |
* 5 |
* 6 |
* 7 |
* 8 |
*   v y
*
* 【提示】你可以使用
* #ifndef _BOTZONE_ONLINE
* 这样的预编译指令来区分在线评测和本地评测
*
* 【提示】一般的文本编辑器都会支持将代码块折叠起来
* 如果你觉得自带代码太过冗长，可以考虑将整个namespace折叠
*/

#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include <stack>
#include<queue>
#include <stdexcept>
#include "jsoncpp/json.h"
//#include <torch/torch.h>

#define FIELD_MAX_HEIGHT 20
#define FIELD_MAX_WIDTH 20
#define MAX_GENERATOR_COUNT 4 // 每个象限1
#define MAX_PLAYER_COUNT 4
#define MAX_TURN 100

// 你也可以选用 using namespace std; 但是会污染命名空间
using std::string;
using std::swap;
using std::cin;
using std::cout;
using std::endl;
using std::getline;
using std::runtime_error;
using std::queue;
using std::sort;

// 平台提供的吃豆人相关逻辑处理程序
namespace Pacman
{
	const time_t seed = time(0);
	const int dx[] = { 0, 1, 0, -1, 1, 1, -1, -1 }, dy[] = { -1, 0, 1, 0, -1, 1, 1, -1 };

	// 枚举定义；使用枚举虽然会浪费空间（sizeof(GridContentType) == 4），但是计算机处理32位的数字效率更高

	// 每个格子可能变化的内容，会采用“或”逻辑进行组合
	enum GridContentType
	{
		empty = 0, // 其实不会用到
		player1 = 1, // 1号玩家
		player2 = 2, // 2号玩家
		player3 = 4, // 3号玩家
		player4 = 8, // 4号玩家
		playerMask = 1 | 2 | 4 | 8, // 用于检查有没有玩家等
		smallFruit = 16, // 小豆子
		largeFruit = 32 // 大豆子
	};

	// 用玩家ID换取格子上玩家的二进制位
	GridContentType playerID2Mask[] = { player1, player2, player3, player4 };
	string playerID2str[] = { "0", "1", "2", "3" };

	// 让枚举也可以用这些运算了（不加会编译错误）
	template<typename T>
	inline T operator |=(T& a, const T& b)
	{
		return a = static_cast<T>(static_cast<int>(a) | static_cast<int>(b));
	}
	template<typename T>
	inline T operator |(const T& a, const T& b)
	{
		return static_cast<T>(static_cast<int>(a) | static_cast<int>(b));
	}
	template<typename T>
	inline T operator &=(T& a, const T& b)
	{
		return a = static_cast<T>(static_cast<int>(a)& static_cast<int>(b));
	}
	template<typename T>
	inline T operator &(const T& a, const T& b)
	{
		return static_cast<T>(static_cast<int>(a)& static_cast<int>(b));
	}
	template<typename T>
	inline T operator -(const T& a, const T& b)
	{
		return static_cast<T>(static_cast<int>(a) - static_cast<int>(b));
	}
	template<typename T>
	inline T operator ++(T& a)
	{
		return a = static_cast<T>(static_cast<int>(a) + 1);
	}
	template<typename T>
	inline T operator ~(const T& a)
	{
		return static_cast<T>(~static_cast<int>(a));
	}

	// 每个格子固定的东西，会采用“或”逻辑进行组合
	enum GridStaticType
	{
		emptyWall = 0, // 其实不会用到
		wallNorth = 1, // 北墙（纵坐标减少的方向）
		wallEast = 2, // 东墙（横坐标增加的方向）
		wallSouth = 4, // 南墙（纵坐标增加的方向）
		wallWest = 8, // 西墙（横坐标减少的方向）
		generator = 16 // 豆子产生器
	};

	// 用移动方向换取这个方向上阻挡着的墙的二进制位
	GridStaticType direction2OpposingWall[] = { wallNorth, wallEast, wallSouth, wallWest };

	// 方向，可以代入dx、dy数组，同时也可以作为玩家的动作
	enum Direction
	{
		stay = -1,
		up = 0,
		right = 1,
		down = 2,
		left = 3,
		shootUp = 4, // 向上发射金光
		shootRight = 5, // 向右发射金光
		shootDown = 6, // 向下发射金光
		shootLeft = 7 // 向左发射金光
	};

	// 场地上带有坐标的物件
	struct FieldProp
	{
		int row, col;
	};

	// 场地上的玩家
	struct Player : FieldProp
	{
		int strength;
		int powerUpLeft;
		bool dead;
		int lastAction;
	};

	// 回合新产生的豆子的坐标
	struct NewFruits
	{
		FieldProp newFruits[MAX_GENERATOR_COUNT * 8];
		int newFruitCount;
	} newFruits[MAX_TURN];
	int newFruitsCount = 0;

	// 状态转移记录结构
	struct TurnStateTransfer
	{
		enum StatusChange // 可组合
		{
			none = 0,
			ateSmall = 1,
			ateLarge = 2,
			powerUpDrop = 4,
			die = 8,
			error = 16
		};

		// 玩家选定的动作
		Direction actions[MAX_PLAYER_COUNT];
		
		// 此回合该玩家的状态变化
		StatusChange change[MAX_PLAYER_COUNT];

		// 此回合该玩家的力量变化
		int strengthDelta[MAX_PLAYER_COUNT];
	};

	// 游戏主要逻辑处理类，包括输入输出、回合演算、状态转移，全局唯一
	class GameField
	{
	private:
		// 为了方便，大多数属性都不是private的

		// 记录每回合的变化（栈）
		TurnStateTransfer backtrack[MAX_TURN];

		// 这个对象是否已经创建
		static bool constructed;

	public:
		// 场地的长和宽
		int height, width;
		int generatorCount;
		int GENERATOR_INTERVAL, LARGE_FRUIT_DURATION, LARGE_FRUIT_ENHANCEMENT, SKILL_COST;

		// 场地格子固定的内容
		GridStaticType fieldStatic[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];

		// 场地格子会变化的内容
		GridContentType fieldContent[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];
		int generatorTurnLeft; // 多少回合后产生豆子
		int aliveCount; // 有多少玩家存活
		int smallFruitCount;
		int turnID;
		FieldProp generators[MAX_GENERATOR_COUNT]; // 有哪些豆子产生器
		Player players[MAX_PLAYER_COUNT]; // 有哪些玩家

		// 玩家选定的动作
		Direction actions[MAX_PLAYER_COUNT];
		//玩家上回合的动作
		int LastActions[MAX_PLAYER_COUNT];

		// 恢复到上次场地状态。可以一路恢复到最开始。
		// 恢复失败（没有状态可恢复）返回false
		bool PopState()
		{
			if (turnID <= 0)
				return false;

			const TurnStateTransfer& bt = backtrack[--turnID];
			int i, _;

			// 倒着来恢复状态

			for (_ = 0; _ < MAX_PLAYER_COUNT; _++)
			{
				Player& _p = players[_];
				GridContentType& content = fieldContent[_p.row][_p.col];
				TurnStateTransfer::StatusChange change = bt.change[_];

				// 5. 大豆回合恢复
				if (change & TurnStateTransfer::powerUpDrop)
					_p.powerUpLeft++;

				// 4. 吐出豆子
				if (change & TurnStateTransfer::ateSmall)
				{
					content |= smallFruit;
					smallFruitCount++;
				}
				else if (change & TurnStateTransfer::ateLarge)
				{
					content |= largeFruit;
					_p.powerUpLeft -= LARGE_FRUIT_DURATION;
				}

				// 2. 魂兮归来
				if (change & TurnStateTransfer::die)
				{
					_p.dead = false;
					aliveCount++;
					content |= playerID2Mask[_];
				}

				// 1. 移形换影
				if (!_p.dead && bt.actions[_] != stay && bt.actions[_] < shootUp)
				{
					fieldContent[_p.row][_p.col] &= ~playerID2Mask[_];
					_p.row = (_p.row - dy[bt.actions[_]] + height) % height;
					_p.col = (_p.col - dx[bt.actions[_]] + width) % width;
					fieldContent[_p.row][_p.col] |= playerID2Mask[_];
				}

				// 0. 救赎不合法的灵魂
				if (change & TurnStateTransfer::error)
				{
					_p.dead = false;
					aliveCount++;
					content |= playerID2Mask[_];
				}

				// *. 恢复力量
				_p.strength -= bt.strengthDelta[_];
			}

			// 3. 收回豆子
			if (generatorTurnLeft == GENERATOR_INTERVAL)
			{
				generatorTurnLeft = 1;
				NewFruits& fruits = newFruits[--newFruitsCount];
				for (i = 0; i < fruits.newFruitCount; i++)
				{
					fieldContent[fruits.newFruits[i].row][fruits.newFruits[i].col] &= ~smallFruit;
					smallFruitCount--;
				}
			}
			else
				generatorTurnLeft++;

			return true;
		}

		// 判断指定玩家向指定方向移动/施放技能是不是合法的（没有撞墙且没有踩到豆子产生器、力量足够）
		inline bool ActionValid(int playerID, Direction& dir) const
		{
			if (dir == stay)
				return true;
			const Player& p = players[playerID];
			if (dir >= shootUp)
				return dir < 8 && p.strength > SKILL_COST;
			return dir >= 0 && dir < 4 &&
				!(fieldStatic[p.row][p.col] & direction2OpposingWall[dir]);
		}

		// 在向actions写入玩家动作后，演算下一回合局面，并记录之前所有的场地状态，可供日后恢复。
		// 是终局的话就返回false
		bool NextTurn()
		{
			int _, i, j;

			TurnStateTransfer& bt = backtrack[turnID];
			memset(&bt, 0, sizeof(bt));

			// 0. 杀死不合法输入
			for (_ = 0; _ < MAX_PLAYER_COUNT; _++)
			{
				Player& p = players[_];
				if (!p.dead)
				{
					Direction& action = actions[_];
					if (action == stay)
						continue;

					if (!ActionValid(_, action))
					{
						bt.strengthDelta[_] += -p.strength;
						bt.change[_] = TurnStateTransfer::error;
						fieldContent[p.row][p.col] &= ~playerID2Mask[_];
						p.strength = 0;
						p.dead = true;
						aliveCount--;
					}
					else if (action < shootUp)
					{
						// 遇到比自己强♂壮的玩家是不能前进的
						GridContentType target = fieldContent
							[(p.row + dy[action] + height) % height]
						[(p.col + dx[action] + width) % width];
						if (target & playerMask)
							for (i = 0; i < MAX_PLAYER_COUNT; i++)
								if (target & playerID2Mask[i] && players[i].strength > p.strength)
									action = stay;
					}
				}
			}

			// 1. 位置变化
			for (_ = 0; _ < MAX_PLAYER_COUNT; _++)
			{
				Player& _p = players[_];

				bt.actions[_] = actions[_];

				if (_p.dead || actions[_] == stay || actions[_] >= shootUp)
					continue;

				// 移动
				fieldContent[_p.row][_p.col] &= ~playerID2Mask[_];
				_p.row = (_p.row + dy[actions[_]] + height) % height;
				_p.col = (_p.col + dx[actions[_]] + width) % width;
				fieldContent[_p.row][_p.col] |= playerID2Mask[_];
			}

			// 2. 玩家互殴
			for (_ = 0; _ < MAX_PLAYER_COUNT; _++)
			{
				Player& _p = players[_];
				if (_p.dead)
					continue;

				// 判断是否有玩家在一起
				int player, containedCount = 0;
				int containedPlayers[MAX_PLAYER_COUNT];
				for (player = 0; player < MAX_PLAYER_COUNT; player++)
					if (fieldContent[_p.row][_p.col] & playerID2Mask[player])
						containedPlayers[containedCount++] = player;

				if (containedCount > 1)
				{
					// NAIVE
					for (i = 0; i < containedCount; i++)
						for (j = 0; j < containedCount - i - 1; j++)
							if (players[containedPlayers[j]].strength < players[containedPlayers[j + 1]].strength)
								swap(containedPlayers[j], containedPlayers[j + 1]);

					int begin;
					for (begin = 1; begin < containedCount; begin++)
						if (players[containedPlayers[begin - 1]].strength > players[containedPlayers[begin]].strength)
							break;

					// 这些玩家将会被杀死
					int lootedStrength = 0;
					for (i = begin; i < containedCount; i++)
					{
						int id = containedPlayers[i];
						Player& p = players[id];

						// 从格子上移走
						fieldContent[p.row][p.col] &= ~playerID2Mask[id];
						p.dead = true;
						int drop = p.strength / 2;
						bt.strengthDelta[id] += -drop;
						bt.change[id] |= TurnStateTransfer::die;
						lootedStrength += drop;
						p.strength -= drop;
						aliveCount--;
					}

					// 分配给其他玩家
					int inc = lootedStrength / begin;
					for (i = 0; i < begin; i++)
					{
						int id = containedPlayers[i];
						Player& p = players[id];
						bt.strengthDelta[id] += inc;
						p.strength += inc;
					}
				}
			}

			// 2.5 金光法器
			for (_ = 0; _ < MAX_PLAYER_COUNT; _++)
			{
				Player& _p = players[_];
				if (_p.dead || actions[_] < shootUp)
					continue;

				_p.strength -= SKILL_COST;
				bt.strengthDelta[_] -= SKILL_COST;

				int r = _p.row, c = _p.col, player;
				Direction dir = actions[_] - shootUp;

				// 向指定方向发射金光（扫描格子直到被挡）
				while (!(fieldStatic[r][c] & direction2OpposingWall[dir]))
				{
					r = (r + dy[dir] + height) % height;
					c = (c + dx[dir] + width) % width;

					// 如果转了一圈回来……
					if (r == _p.row && c == _p.col)
						break;

					if (fieldContent[r][c] & playerMask)
						for (player = 0; player < MAX_PLAYER_COUNT; player++)
							if (fieldContent[r][c] & playerID2Mask[player])
							{
								players[player].strength -= SKILL_COST * 1.5;
								bt.strengthDelta[player] -= SKILL_COST * 1.5;
								_p.strength += SKILL_COST * 1.5;
								bt.strengthDelta[_] += SKILL_COST * 1.5;
							}
				}
			}

			// *. 检查一遍有无死亡玩家
			for (_ = 0; _ < MAX_PLAYER_COUNT; _++)
			{
				Player& _p = players[_];
				if (_p.dead || _p.strength > 0)
					continue;

				// 从格子上移走
				fieldContent[_p.row][_p.col] &= ~playerID2Mask[_];
				_p.dead = true;

				// 使其力量变为0
				bt.strengthDelta[_] += -_p.strength;
				bt.change[_] |= TurnStateTransfer::die;
				_p.strength = 0;
				aliveCount--;
			}


			// 3. 产生豆子
			if (--generatorTurnLeft == 0)
			{
				generatorTurnLeft = GENERATOR_INTERVAL;
				NewFruits& fruits = newFruits[newFruitsCount++];
				fruits.newFruitCount = 0;
				for (i = 0; i < generatorCount; i++)
					for (Direction d = up; d < 8; ++d)
					{
						// 取余，穿过场地边界
						int r = (generators[i].row + dy[d] + height) % height, c = (generators[i].col + dx[d] + width) % width;
						if (fieldStatic[r][c] & generator || fieldContent[r][c] & (smallFruit | largeFruit))
							continue;
						fieldContent[r][c] |= smallFruit;
						fruits.newFruits[fruits.newFruitCount].row = r;
						fruits.newFruits[fruits.newFruitCount++].col = c;
						smallFruitCount++;
					}
			}

			// 4. 吃掉豆子
			for (_ = 0; _ < MAX_PLAYER_COUNT; _++)
			{
				Player& _p = players[_];
				if (_p.dead)
					continue;

				GridContentType& content = fieldContent[_p.row][_p.col];

				// 只有在格子上只有自己的时候才能吃掉豆子
				if (content & playerMask & ~playerID2Mask[_])
					continue;

				if (content & smallFruit)
				{
					content &= ~smallFruit;
					_p.strength++;
					bt.strengthDelta[_]++;
					smallFruitCount--;
					bt.change[_] |= TurnStateTransfer::ateSmall;
				}
				else if (content & largeFruit)
				{
					content &= ~largeFruit;
					if (_p.powerUpLeft == 0)
					{
						_p.strength += LARGE_FRUIT_ENHANCEMENT;
						bt.strengthDelta[_] += LARGE_FRUIT_ENHANCEMENT;
					}
					_p.powerUpLeft += LARGE_FRUIT_DURATION;
					bt.change[_] |= TurnStateTransfer::ateLarge;
				}
			}

			// 5. 大豆回合减少
			for (_ = 0; _ < MAX_PLAYER_COUNT; _++)
			{
				Player& _p = players[_];
				if (_p.dead)
					continue;

				if (_p.powerUpLeft > 0)
				{
					bt.change[_] |= TurnStateTransfer::powerUpDrop;
					if (--_p.powerUpLeft == 0)
					{
						_p.strength -= LARGE_FRUIT_ENHANCEMENT;
						bt.strengthDelta[_] += -LARGE_FRUIT_ENHANCEMENT;
					}
				}
			}

			// *. 检查一遍有无死亡玩家
			for (_ = 0; _ < MAX_PLAYER_COUNT; _++)
			{
				Player& _p = players[_];
				if (_p.dead || _p.strength > 0)
					continue;

				// 从格子上移走
				fieldContent[_p.row][_p.col] &= ~playerID2Mask[_];
				_p.dead = true;

				// 使其力量变为0
				bt.strengthDelta[_] += -_p.strength;
				bt.change[_] |= TurnStateTransfer::die;
				_p.strength = 0;
				aliveCount--;
			}

			++turnID;

			// 是否只剩一人？
			if (aliveCount <= 1)
			{
				for (_ = 0; _ < MAX_PLAYER_COUNT; _++)
					if (!players[_].dead)
					{
						bt.strengthDelta[_] += smallFruitCount;
						players[_].strength += smallFruitCount;
					}
				return false;
			}

			// 是否回合超限？
			if (turnID >= 100)
				return false;

			return true;
		}

		// 读取并解析程序输入，本地调试或提交平台使用都可以。
		// 如果在本地调试，程序会先试着读取参数中指定的文件作为输入文件，失败后再选择等待用户直接输入。
		// 本地调试时可以接受多行以便操作，Windows下可以用 Ctrl-Z 或一个【空行+回车】表示输入结束，但是在线评测只需接受单行即可。
		// localFileName 可以为NULL
		// obtainedData 会输出自己上回合存储供本回合使用的数据
		// obtainedGlobalData 会输出自己的 Bot 上以前存储的数据
		// 返回值是自己的 playerID
		int ReadInput(const char* localFileName, string& obtainedData, string& obtainedGlobalData)
		{
			string str, chunk;
#ifdef _BOTZONE_ONLINE
			std::ios::sync_with_stdio(false); //ω\\)
			getline(cin, str);
#else
			if (localFileName)
			{
				std::ifstream fin(localFileName);
				if (fin)
					while (getline(fin, chunk) && chunk != "")
						str += chunk;
				else
					while (getline(cin, chunk) && chunk != "")
						str += chunk;
			}
			else
				while (getline(cin, chunk) && chunk != "")
					str += chunk;
#endif
			Json::Reader reader;
			Json::Value input;
			reader.parse(str, input);

			int len = input["requests"].size();

			// 读取场地静态状况
			Json::Value field = input["requests"][(Json::Value::UInt) 0],
				staticField = field["static"], // 墙面和产生器
				contentField = field["content"]; // 豆子和玩家
			height = field["height"].asInt();
			width = field["width"].asInt();
			LARGE_FRUIT_DURATION = field["LARGE_FRUIT_DURATION"].asInt();
			LARGE_FRUIT_ENHANCEMENT = field["LARGE_FRUIT_ENHANCEMENT"].asInt();
			SKILL_COST = field["SKILL_COST"].asInt();
			generatorTurnLeft = GENERATOR_INTERVAL = field["GENERATOR_INTERVAL"].asInt();

			

			PrepareInitialField(staticField, contentField);

			// 根据历史恢复局面
			for (int i = 1; i < len; i++)
			{
				Json::Value req = input["requests"][i];
				for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
					if (!players[_].dead)
						actions[_] = (Direction)req[playerID2str[_]]["action"].asInt();
				NextTurn();
			}

			for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
			{
				players[_].lastAction = actions[_];
				LastActions[_] = actions[_];//储存上回合的动作
			}

			obtainedData = input["data"].asString();
			
			obtainedGlobalData = input["globaldata"].asString();

			return field["id"].asInt();
		}

		// 根据 static 和 content 数组准备场地的初始状况
		void PrepareInitialField(const Json::Value& staticField, const Json::Value& contentField)
		{
			int r, c, gid = 0;
			generatorCount = 0;
			aliveCount = 0;
			smallFruitCount = 0;
			generatorTurnLeft = GENERATOR_INTERVAL;
			for (r = 0; r < height; r++)
				for (c = 0; c < width; c++)
				{
					GridContentType& content = fieldContent[r][c] = (GridContentType)contentField[r][c].asInt();
					GridStaticType& s = fieldStatic[r][c] = (GridStaticType)staticField[r][c].asInt();
					if (s & generator)
					{
						generators[gid].row = r;
						generators[gid++].col = c;
						generatorCount++;
					}
					if (content & smallFruit)
						smallFruitCount++;
					for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
						if (content & playerID2Mask[_])
						{
							Player& p = players[_];
							p.col = c;
							p.row = r;
							p.powerUpLeft = 0;
							p.strength = 1;
							p.dead = false;
							aliveCount++;
						}
				}
		}

		// 完成决策，输出结果。
		// action 表示本回合的移动方向，stay 为不移动，shoot开头的动作表示向指定方向施放技能
		// tauntText 表示想要叫嚣的言语，可以是任意字符串，除了显示在屏幕上不会有任何作用，留空表示不叫嚣
		// data 表示自己想存储供下一回合使用的数据，留空表示删除
		// globalData 表示自己想存储供以后使用的数据（替换），这个数据可以跨对局使用，会一直绑定在这个 Bot 上，留空表示删除
		void WriteOutput(Direction action, string tauntText = "", string data = "", string globalData = "") const
		{
			Json::Value ret;
			ret["response"]["action"] = action;
			ret["response"]["tauntText"] = tauntText;
			ret["data"] = data;
			ret["globaldata"] = globalData;
			ret["debug"] = (Json::Int)seed;

#ifdef _BOTZONE_ONLINE
			Json::FastWriter writer; // 在线评测的话能用就行……
#else
			Json::StyledWriter writer; // 本地调试这样好看 > <
#endif
			cout << writer.write(ret) << endl;
		}

		// 用于显示当前游戏状态，调试用。
		// 提交到平台后会被优化掉。
		inline void DebugPrint() const
		{
#ifndef _BOTZONE_ONLINE
			printf("回合号【%d】存活人数【%d】| 图例 产生器[G] 有玩家[0/1/2/3] 多个玩家[*] 大豆[o] 小豆[.]\n", turnID, aliveCount);
			for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
			{
				const Player& p = players[_];
				printf("[玩家%d(%d, %d)|力量%d|加成剩余回合%d|%s]\n",
					_, p.row, p.col, p.strength, p.powerUpLeft, p.dead ? "死亡" : "存活");
			}
			putchar(' ');
			putchar(' ');
			for (int c = 0; c < width; c++)
				printf("  %d ", c);
			putchar('\n');
			for (int r = 0; r < height; r++)
			{
				putchar(' ');
				putchar(' ');
				for (int c = 0; c < width; c++)
				{
					putchar(' ');
					printf((fieldStatic[r][c] & wallNorth) ? "---" : "   ");
				}
				printf("\n%d ", r);
				for (int c = 0; c < width; c++)
				{
					putchar((fieldStatic[r][c] & wallWest) ? '|' : ' ');
					putchar(' ');
					int hasPlayer = -1;
					for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
						if (fieldContent[r][c] & playerID2Mask[_])
							if (hasPlayer == -1)
								hasPlayer = _;
							else
								hasPlayer = 4;
					if (hasPlayer == 4)
						putchar('*');
					else if (hasPlayer != -1)
						putchar('0' + hasPlayer);
					else if (fieldStatic[r][c] & generator)
						putchar('G');
					else if (fieldContent[r][c] & playerMask)
						putchar('*');
					else if (fieldContent[r][c] & smallFruit)
						putchar('.');
					else if (fieldContent[r][c] & largeFruit)
						putchar('o');
					else
						putchar(' ');
					putchar(' ');
				}
				putchar((fieldStatic[r][width - 1] & wallEast) ? '|' : ' ');
				putchar('\n');
			}
			putchar(' ');
			putchar(' ');
			for (int c = 0; c < width; c++)
			{
				putchar(' ');
				printf((fieldStatic[height - 1][c] & wallSouth) ? "---" : "   ");
			}
			putchar('\n');
#endif
		}

		Json::Value SerializeCurrentTurnChange()
		{
			Json::Value result;
			TurnStateTransfer& bt = backtrack[turnID - 1];
			for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
			{
				result["actions"][_] = bt.actions[_];
				result["strengthDelta"][_] = bt.strengthDelta[_];
				result["change"][_] = bt.change[_];
			}
			return result;
		}

		// 初始化游戏管理器
		GameField()
		{
			if (constructed)
				throw runtime_error("请不要再创建 GameField 对象了，整个程序中只应该有一个对象");
			constructed = true;

			turnID = 0;
		}

		GameField(const GameField& b) : GameField() { }
	};

	bool GameField::constructed = false;
}



namespace Bot
{
	const int infDis = 0x3fff;
	struct location
	{
		int x, y;
		location() { x = 0; y = 0; }
		location(int x, int y) :x(x), y(y) {}
	};
	struct justify
	{
		int dis;
		int dir;
		bool isDanger;
		int value;
		justify() { dis = infDis; dir = -1; isDanger = false; value = 0; }
		justify(int x, int y) :dis(x), dir(y) { isDanger = false; value = 0; }
	};

	struct valDou
	{
		double sum;//豆子分值
		int x;
		int y;
		int big;//是否为大豆子
		int dist;//距离当前点的距离
		int numNear;//距离接近的豆子数
		int totNear;//距离接近的豆子总距离
		int Nearest;//从当前豆子出发到达豆子距离接近的所有豆子的最短路径
	};

	struct Node//结点
	{
		int u;
		int v;
		int w;
	}Edge[160005];


	const int Near = 3;//考虑两个豆子是否是接近的距离标准，待定


	int vis[20][20] = {};//是否访问过
	justify valueMap[20][20] = {};//记录BFS的结果
	int mustShootData[4] = {0,0,0,0};//记录当前位置四个方向上必定能打到的人数，排序：上 右 下 左
	int shootData[4] = {0,0,0,0};//记录当前位置四个方向上能打到的人数，排序：上 右 下 左
	//最优豆子信息
	int douDis = infDis, douDir = -1;
	//最优豆子产生器附近信息
	int genDis = infDis, genDir = -1;
	//最终决策
	int final = -1;
	//叫嚣语句
	string shoutString = "";


	justify valueALL[20][20][20][20] = {};//记录全图BFS结果
	int visALL[20][20] = {};
	

	//便于本地调试，每次都初始化
	void init(Pacman::GameField& gameField) {
		final = -1;
		douDis = infDis, douDir = -1;
		genDis = infDis, genDir = -1;
		for (int i = 0; i < 4; i++) {
			mustShootData[i] = 0;
			shootData[i] = 0;
		}
		for (int x = 0; x < gameField.height; x++)
		{
			for (int y = 0; y < gameField.width; y++) 
			{
				vis[x][y] = 0;
				valueMap[x][y].dis=infDis;
				valueMap[x][y].dir = -1;

				valueMap[x][y].isDanger = false;
				valueMap[x][y].value = 0;
			}
		}
	}
	//判定危险范围
	void dangerJustify(Pacman::GameField& gameField, int myID) 
	{
		for (int player = 0; player < MAX_PLAYER_COUNT; player++) 
		{
			//自己当然不危险
			if (player == myID) continue;
			Pacman::Player& aPlayer = gameField.players[player];
			if (gameField.fieldContent[aPlayer.row][aPlayer.col] & Pacman::playerID2Mask[player])
				{
				if (gameField.players[player].strength > gameField.players[myID].strength) 
				{
					//不能直接走到人家嘴里去
					valueMap[aPlayer.row][aPlayer.col].isDanger = true;
					//把坏家伙旁边的可达点也设为危险，避免傻屌行为暴毙
					for (int dir = 0; dir < 4; dir++) {
						if (!(gameField.fieldStatic[aPlayer.row][aPlayer.col] & Pacman::direction2OpposingWall[(Pacman::Direction)(dir)])) {
								valueMap
									[(aPlayer.row + Pacman::dy[dir] + gameField.height) % gameField.height]
									[(aPlayer.col + Pacman::dx[dir]+ gameField.width) % gameField.width].isDanger = true;
						}
					}
				}
	
			}
		}
	}

	//预处理图上任意两点之间距离及第一步方向
	void BFSALL(Pacman::GameField& gameField)
	{
		for (int I = 0; I < gameField.height; I++)
		{
			for (int J = 0; J < gameField.width; J++)
			{
				queue <location>q;
				while (!q.empty()) q.pop();
				memset(visALL, 0, sizeof(visALL));
				visALL[I][J] = 1;
				q.push(location(I, J));
				if (!valueALL[I][J][I][J].isDanger)
					valueALL[I][J][I][J].dis = 0;
				location u, v;
				while (!q.empty())
				{
					u = q.front();
					q.pop();
					for (int i = 0; i < 4; i++)
					{
						v.x = u.x + Pacman::dy[i];
						v.y = u.y + Pacman::dx[i];
						//边界判定
						if (v.x < 0) v.x += gameField.height;
						if (v.x >= gameField.height) v.x -= gameField.height;
						if (v.y < 0) v.y += gameField.width;
						if (v.y >= gameField.width) v.y -= gameField.width;
						//(u.x,u.y)到(v.x,v.y)方向上没有墙，(v.x,v.y)不是豆子产生器，(v.x,v.y)没走过
						if (!(gameField.fieldStatic[u.x][u.y] & Pacman::direction2OpposingWall[(Pacman::Direction)(i)])
							&& !visALL[v.x][v.y])
						{
							q.push(v);
							visALL[v.x][v.y] = 1;
							//对于初始点要特殊处理
							if (u.x == I && u.y == J) valueALL[I][J][v.x][v.y].dis = 1;
							else valueALL[I][J][v.x][v.y].dis = valueALL[I][J][u.x][u.y].dis + 1;
							if (u.x == I && u.y == J) valueALL[I][J][v.x][v.y].dir = i;//起始点方向记录
							else valueALL[I][J][v.x][v.y].dir = valueALL[I][J][u.x][u.y].dir;//传递方向
						}
					}
				}
			}
		}
	}

	//BFS遍历图上所有点
	void BFSd(Pacman::GameField& gameField, int myID, int nowx, int nowy)
	{
		queue <location>q;
		while (!q.empty()) q.pop();
		memset(vis, 0, sizeof(vis));
		vis[nowx][nowy] = 1;
		q.push(location(nowx, nowy));
		if(!valueMap[nowx][nowy].isDanger)
		valueMap[nowx][nowy].dis = 0;
		location u, v;
		while (!q.empty())
		{
			u = q.front();
			q.pop();
			for (int i = 0; i < 4; i++)
			{
				v.x = u.x + Pacman::dy[i];
				v.y = u.y + Pacman::dx[i];
				//边界判定
				if (v.x < 0) v.x += gameField.height;
				if (v.x >= gameField.height) v.x -= gameField.height;
				if (v.y < 0) v.y += gameField.width;
				if (v.y >= gameField.width) v.y -= gameField.width;

				//(u.x,u.y)到(v.x,v.y)方向上没有墙，(v.x,v.y)不是豆子产生器，(v.x,v.y)没走过
				if (!(gameField.fieldStatic[u.x][u.y] & Pacman::direction2OpposingWall[(Pacman::Direction)(i)]) 
					&& !vis[v.x][v.y])
				{
					//危险地带当然不能走，直接跳过可以理解成让程序绕路
					if (valueMap[v.x][v.y].isDanger) continue;
					q.push(v);
					vis[v.x][v.y] = 1;
					//对于初始点要特殊处理，否则当所在点为危险时，其他所有点都是inf啦
					if (u.x == nowx && u.y == nowy) valueMap[v.x][v.y].dis = 1;
					else valueMap[v.x][v.y].dis = valueMap[u.x][u.y].dis + 1;

					if (u.x == nowx && u.y == nowy) valueMap[v.x][v.y].dir = i;//起始点方向记录
					else valueMap[v.x][v.y].dir = valueMap[u.x][u.y].dir;//传递方向
				}
			}
		}
	}

	//遍历四个方向看看有没有人可以打
	int Shot(Pacman::GameField& gameField, int myID, string& shoutString, Pacman::Player& me, int final)
	{
		int shootDir = -1;
		int maxTargetNum = 0;
		for (int dir = 0; dir < 4; dir++) {
			int r = me.row;
			int c = me.col;
			int distance = 0;
			//如果不开枪下回合自己的位置
			int nextR = me.row;
			int nextC = me.col;
			if (final != -1) {
				nextR = (me.row + Pacman::dy[final] + gameField.height) % gameField.height;
				nextC = (me.col + Pacman::dx[final] + gameField.width) % gameField.width;
			}
			while (!(gameField.fieldStatic[r][c] & Pacman::direction2OpposingWall[dir]))
			{
				distance++;
				r = (r + Pacman::dy[dir] + gameField.height) % gameField.height;
				c = (c + Pacman::dx[dir] + gameField.width) % gameField.width;

				// 如果转了一圈回来……
				if (r == me.row && c == me.col)
					break;
				if (gameField.fieldContent[r][c] & Pacman::playerMask)
					for (int player = 0; player < MAX_PLAYER_COUNT; player++)
						if (player != myID && (gameField.fieldContent[r][c] & Pacman::playerID2Mask[player]))
						{
							//记录自己是不是躲不开
							bool canNotHide = (gameField.fieldStatic[me.row][me.col] & Pacman::direction2OpposingWall[(dir + 1) % 4])
								&& (gameField.fieldStatic[me.row][me.col] & Pacman::direction2OpposingWall[(dir + 3) % 4]);

							//记录下回合是不是也躲不开
							bool nextCanNotHide = (final == -1)
								||
								((final == dir || final == (dir + 2) % 4)
									&& (gameField.fieldStatic[nextR][nextC] & Pacman::direction2OpposingWall[(dir + 1) % 4])
									&& (gameField.fieldStatic[nextR][nextC] & Pacman::direction2OpposingWall[(dir + 3) % 4]));
							//先记录此方向有人
							shootData[dir]++;
							//检测此玩家是不是跑不掉了
							if (
								(gameField.fieldStatic[r][c] & Pacman::direction2OpposingWall[(dir + 1) % 4])
								&& (gameField.fieldStatic[r][c] & Pacman::direction2OpposingWall[(dir + 3) % 4])
								) {
								//如果自己也没路跑，并且对方比自己强大或势均力敌，为避免互射致死，走为上计
								if ((gameField.fieldStatic[me.row][me.col] & Pacman::direction2OpposingWall[(Pacman::Direction)(dir + 1) % 4])
									&& (gameField.fieldStatic[me.row][me.col] & Pacman::direction2OpposingWall[(Pacman::Direction)(dir + 1) % 4])
									&& gameField.players[player].strength > gameField.players[myID].strength) {
								}
								else
								mustShootData[dir]++;
							}
							//判断是否很大机会命中
							//如果我们没得躲,且对方比较弱，那就对射
							else if (canNotHide
								&& gameField.players[player].strength <= gameField.players[myID].strength
								) {
								mustShootData[dir]++;
							}
							//如果下回合也躲不开，那不管咋样都得打
							else if (canNotHide && nextCanNotHide) {
								mustShootData[dir]++;
							}
							//如果上回合没动，假定他这回合如果不生成果子也不动
							else if (
								(gameField.players[player].lastAction == -1 && gameField.generatorTurnLeft != 0)
								) {
								mustShootData[dir]++;
							}
							//以下两个判断对方是否刚刚拐进小巷，拐进来的就是送上门啦（除了某些丧心病狂的程序还会躲
							else if (
								(gameField.fieldStatic[r][c] & Pacman::direction2OpposingWall[(dir + 1) % 4])
								&&distance>1
								&& gameField.players[player].lastAction == (dir+1)%4
								) {
								mustShootData[dir]++;
							}
							else if (
								(gameField.fieldStatic[r][c] & Pacman::direction2OpposingWall[(dir + 3) % 4])
								&& distance > 1
								&& gameField.players[player].lastAction == (dir +3) % 4){
								mustShootData[dir]++;
							}
						}
			}
			//挑出目标最多的方向打
			if (mustShootData[dir] > maxTargetNum) {
				shootDir = dir + 4;
				maxTargetNum = mustShootData[dir];
			}
		}
		//有人可以打当然要打啦(前提是不处在危险之中)
		if (shootDir != -1 && (gameField.SKILL_COST < gameField.players[myID].strength)) {
			if (!valueMap[me.row][me.col].isDanger) {
				final = shootDir;
				shoutString = "我射";
			}
		}
		//如果到了死路，路被封死了
		if (final == -1 && valueMap[me.row][me.col].isDanger) {
			//能跑先跑
			for (int dir = 0; dir < 4; dir++) {
				if ((gameField.fieldStatic[me.row][me.col] & Pacman::direction2OpposingWall[dir]) &&
					(valueMap[(me.row + Pacman::dy[dir] + gameField.height) % gameField.height][(me.col + Pacman::dx[dir] + gameField.width) % gameField.width].dis != infDis)) {
					final = dir;
				}
			}
			//实在不能跑就拼啦
			if (final == -1 && (gameField.SKILL_COST < gameField.players[myID].strength)) {
				int maxShootNum = 0;
				for (int dir = 0; dir < 4; dir++) {
					if (maxShootNum < shootData[dir]) {
						maxShootNum = shootData[dir];
						shootDir = dir + 4;
						shoutString = "跟你拼啦！";
					}
				}
				final = shootDir;
			}
		}
		return final;
	}
	//吃豆子

	bool cmp(Node x ,Node y)
	{
		return x.w < y.w;
	}
	void predictPlayers(Pacman::GameField& gameField, int myID) {

	}
	int Eat(Pacman::GameField& gameField, int myID, string& shoutString, int final, int& douDis, int& douDir, int& genDis, int& genDir, int X, int Y)
	{
		valDou tot[405];
		int NumDou = 0; location AnsEat;//可达豆子总数，最终选择吃的豆子坐标
		for (int i = 1; i <= 400; i++) tot[i].x = tot[i].y = tot[i].big = tot[i].dist = tot[i].numNear = tot[i].totNear = tot[i].sum = 0;

		/*
		//枚举图上所有点找最近的豆子
		for (int i = 0; i < gameField.height; i++)
			for (int j = 0; j < gameField.width; j++)
			{

				//暂时大果子小果子都要，并且判定是否有人重叠于果子上
				if (((gameField.fieldContent[i][j] & Pacman::smallFruit)
					|| (gameField.fieldContent[i][j] & Pacman::largeFruit))
					&& (!(gameField.fieldContent[i][j] & Pacman::playerMask)))
					if ((douDis >= valueALL[X][Y][i][j].dis) && !(valueALL[X][Y][i][j].dis == infDis))
					{
						if (douDis > valueALL[X][Y][i][j].dis) {
							douDis = valueALL[X][Y][i][j].dis;
							douDir = valueALL[X][Y][i][j].dir;
						}
						//如果遇到一样近的，那么随机选择要不要选择此果实，防止出现两人一直重叠
						else {
							if (rand() % 2)
							{
								douDis = valueALL[X][Y][i][j].dis;
								douDir = valueALL[X][Y][i][j].dir;
							}
						}
					}
			}
			*/
		
			
		//枚举图上所有点先找出所有可达的豆子
		for (int i = 0; i < gameField.height; i++)
			for (int j = 0; j < gameField.width; j++)
			{
				if (((gameField.fieldContent[i][j] & Pacman::smallFruit)
					|| (gameField.fieldContent[i][j] & Pacman::largeFruit))
					&&(!valueMap[i][j].isDanger)
					&& (!(gameField.fieldContent[i][j] & Pacman::playerMask)))
				{
					++NumDou;
					if (gameField.fieldContent[i][j] & Pacman::largeFruit) tot[NumDou].big = 1;
					tot[NumDou].x = i; tot[NumDou].y = j; tot[NumDou].dist = valueMap[i][j].dis;
				}
			}
		//再对每个豆子进行价值计算，考虑的因素包括与当前点距离，豆子周围“接近”的豆子数量
		for (int i = 1; i < NumDou; i++)
		{
			for (int j = i+1; j <= NumDou; j++)
			{
				if (valueALL[tot[i].x][tot[i].y][tot[j].x][tot[j].y].dis <= Near)
				{
					tot[i].numNear++; tot[j].numNear++;
					tot[i].totNear += valueALL[tot[i].x][tot[i].y][tot[j].x][tot[j].y].dis; 
					tot[j].totNear += valueALL[tot[i].x][tot[i].y][tot[j].x][tot[j].y].dis;
				}
			}
		}

		//计算从目标豆子出发到达所有距离接近的豆子的最短路径（即求一个最小生成树）
		int Node[405], NumNode=0,fNode[405];
		for (int i = 1; i <= NumDou; i++)
		{
			memset(Edge, 0, sizeof(Edge));
			memset(Node, 0, sizeof(Node));
			int NumEdge = 0, tmp = 0;
			NumNode = 0;
			Node[++NumNode] = i;
			for (int j = 1; j <= NumNode; j++)
			{
				fNode[j] = j;//标记集合
			}
			for (int j = 1; j <= NumDou; j++)
			{
				if (i == j) continue;
				if (valueALL[tot[i].x][tot[i].y][tot[j].x][tot[j].y].dis <= Near) Node[++NumNode] = j;
			}

			for (int j = 1; j <= NumNode; j++)
			{
				for (int k = j + 1; k <= NumNode; k++)
				{
					Edge[++NumEdge].u = Node[j];
					Edge[NumEdge].v = Node[k];
					Edge[NumEdge].w = valueALL[tot[Node[j]].x][tot[Node[j]].y][tot[Node[k]].x][tot[Node[k]].y].dis;
				}
			}

			sort(Edge + 1, Edge + NumEdge + 1, cmp);

			for (int j = 1; j <= NumEdge; j++)
			{
				int n1 = fNode[Edge[j].u];
				int n2 = fNode[Edge[j].v];//得到两点集合编号
				if (n1 != n2)//不同就加入边
				{
					tot[i].Nearest += Edge[j].w;
					for (int k = 1; k <= NumNode; k++)
					{
						if (fNode[k] == n2) fNode[k] = n1;//合并集合
					}
					tmp++;
				}
				if (tmp == NumNode - 1) break;
			}
		}
			

		for (int i = 1; i <= NumDou; i++)
		{
			tot[i].sum = tot[i].numNear - tot[i].dist + 0.01 * tot[i].big;//分值为距离接近的豆子数-距离，认为同等情况下大豆子优于小豆子
		}
		for (int i = 1; i <= NumDou; i++)//对豆子价值进行排序
		{
			for (int j = i + 1; j <= NumDou; j++)
			{
				if (tot[i].sum < tot[j].sum)  swap(tot[i], tot[j]);
			}
		}
		if (tot[1].dist != infDis)
		{
			douDis = valueALL[X][Y][tot[1].x][tot[1].y].dis;
			douDir = valueALL[X][Y][tot[1].x][tot[1].y].dir;
		}

		//暂时先决定去吃豆子
		if (douDir != infDis) {
			shoutString = "恰恰恰~";
			final = douDir;
		}
		if (douDis == infDis)
		{
			//找最近的豆子产生器（先粗略找一下,只有场地上没豆子的时候会去找）
			for (int i = 0; i < MAX_GENERATOR_COUNT; i++)
			{
				justify* generatorSide[4];
				generatorSide[0] = &(valueALL[X][Y][gameField.generators[i].row][(gameField.generators[i].col + gameField.width - 1) % gameField.width]);
				generatorSide[1] = &(valueALL[X][Y][gameField.generators[i].row ][(gameField.generators[i].col + 1 + gameField.width) % gameField.width]);
				generatorSide[2] = &(valueALL[X][Y][(gameField.generators[i].row + 1 + gameField.height) % gameField.height][(gameField.generators[i].col)]);
				generatorSide[3] = &(valueALL[X][Y][(gameField.generators[i].row - 1 + gameField.height) % gameField.height][(gameField.generators[i].col)]);
				for (int pos = 0; pos < 4; pos++)
				{
					if (genDis > (*generatorSide[pos]).dis)
					{
						genDis = (*generatorSide[pos]).dis;
						genDir = (*generatorSide[pos]).dir;
					}
				}
			}
			//没豆子吃辣。那就去等着
			if (genDis != infDis) {
				shoutString = "还有的吃";
				final = genDir;
			}
		}
		return final;
	}

	int calc(Pacman::GameField& gameField, int myID)
	{
		init(gameField);
		Pacman::Player& me = gameField.players[myID];
		
		dangerJustify(gameField, myID);
		BFSd(gameField, myID, me.row, me.col);
		//遍历全图预处理
		BFSALL(gameField);

		final = Eat(gameField, myID, shoutString, final,douDis,douDir,genDis,genDir,me.row,me.col);
		
		final = Shot(gameField, myID, shoutString, me, final);


		//最后一道防线，防止出现违规输出
		if (final >= -1 && final <= 7)
		{
			if (final == -1) shoutString = "敌不动我不动敌";
			return final;
		}
		else {
			shoutString = "bugggg了";
			return -1;
		}
	}

	
	string shout() {
		return shoutString;
	}
}

// 一些辅助程序
namespace Helpers
{

	double actionScore[9] = {};


	void LocalPlay
	(Pacman::GameField& gameField, int myID,int countLimit)
	{
		int count = 0, myAct = -1;
		while (true)
		{
			for (int i = 0; i < MAX_PLAYER_COUNT; i++)
			{
				if (gameField.players[i].dead)
					continue;
				gameField.actions[i] = (Pacman::Direction)Bot::calc(gameField, i);;
			}

			if (count == 0)
				myAct = gameField.actions[myID];

			// 演算一步局面变化
			gameField.DebugPrint();
			// NextTurn返回true表示游戏没有结束
			bool hasNext = gameField.NextTurn();
			count++;
			cout << count << endl;
			//限制下最大回合数
			if (count >= countLimit) {
				break;
			}
			if (!hasNext)
				break;
		}

		// 计算分数

		int total = 0;
		for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
			total += gameField.players[_].strength;

		if (total != 0)
			actionScore[myAct + 1] += (10000 * gameField.players[myID].strength / total) / 100.0;

		// 恢复游戏状态到最初（就是本回合）
		while (count-- > 0)
			gameField.PopState();
	}
}


//struct Net : torch::nn::Module {
//	Net() {
//		// Construct and register two Linear submodules.
//		fc1 = register_module("fc1", torch::nn::Linear(2433, 2048));
//		fc2 = register_module("fc2", torch::nn::Linear(2048, 1024));
//		fc3 = register_module("fc3", torch::nn::Linear(1024, 128));
//		fc4 = register_module("fc4", torch::nn::Linear(128,9))
//	}
//
//	// Implement the Net's algorithm.
//	torch::Tensor forward(torch::Tensor x) {
//		// Use one of many tensor manipulation functions.
//		x = torch::relu(fc1->forward(x.reshape({ x.size(0), 784 })));
//		x = torch::dropout(x, /*p=*/0.5, /*train=*/is_training());
//		x = torch::relu(fc2->forward(x));
//		x = torch::log_softmax(fc3->forward(x), /*dim=*/1);
//		return x;
//	}
//
//	// Use one of many "standard library" modules.
//	torch::nn::Linear fc1{ nullptr }, fc2{ nullptr }, fc3{ nullptr }, fc4{ nullptr };
//};




int main()
{
	Pacman::GameField gameField;
	string data, globalData; 
	int myID = gameField.ReadInput("input.txt", data, globalData); // 输入，并获得自己ID
	/*for (int i = 0; i < 4; i++)
		cout << " " << gameField.LastActions[i] << endl;*/
	srand(Pacman::seed + myID);
#ifdef _BOTZONE_ONLINE
	int Ans = Bot::calc(gameField, myID);


	// 输出当前游戏局面状态以供本地调试。注意提交到平台上会自动优化掉，不必担心。
	gameField.DebugPrint();

	gameField.WriteOutput((Pacman::Direction)(Ans), Bot::shout(), data, globalData);
#else
	//调试用，本地模拟
	//Helpers::LocalPlay(gameField, myID,10);
	int Ans=(Pacman::Direction)Bot::calc(gameField, myID);

	gameField.DebugPrint();
	
	gameField.WriteOutput((Pacman::Direction)(Ans), Bot::shout(), data, globalData);
#endif


	
	
	return 0;
}

