#include <string>
#include "jsoncpp/json.h"

#define FIELD_MAX_HEIGHT 20
#define FIELD_MAX_WIDTH 20
#define MAX_GENERATOR_COUNT 4 // ÿ������1
#define MAX_PLAYER_COUNT 4
#define MAX_TURN 103
#define MAX_ACTION_COUNT 9

// ƽ̨�ṩ�ĳԶ�������߼��������
namespace Pacman
{
    using namespace std;

    extern time_t seed;

    // ��ö��Ҳ��������Щ�����ˣ����ӻ�������
    template<typename T>
    T operator |=(T& a, const T& b);
    template<typename T>
    T operator |(const T& a, const T& b);
    template<typename T>
    T operator &=(T& a, const T& b);
    template<typename T>
    T operator &(const T& a, const T& b);
    template<typename T>
    T operator -(const T& a, const T& b);
    template<typename T>
    T operator ++(T& a);
    template<typename T>
    T operator ~(const T& a);

    constexpr int dx[8] = { 0, 1, 0, -1, 1, 1, -1, -1 }, dy[8] = { -1, 0, 1, 0, -1, 1, 1, -1 };
    /* ʹ��dx,dyǰ��ע�⣡��������������������botzone�ٷ�����ϰ���������ģ���������
      +------------->x
      |
      |
      |
      |
      |
      |
      v
      y

      �����ϰ������
      +------------->y
      |
      |
      |
      |
      |
      |
      v
      x

      ��ʹ�������dir����
     */
    constexpr int dir[4][2] = { {-1, 0}, {0, 1}, {1, 0}, {0, -1} };

    // ö�ٶ��壻ʹ��ö����Ȼ���˷ѿռ䣨sizeof(GridContentType) == 4�������Ǽ��������32λ������Ч�ʸ���

    // ÿ�����ӿ��ܱ仯�����ݣ�����á����߼��������
    enum GridContentType
    {
        empty = 0, // ��ʵ�����õ�
        player1 = 1, // 1�����
        player2 = 2, // 2�����
        player3 = 4, // 3�����
        player4 = 8, // 4�����
        playerMask = 1 | 2 | 4 | 8, // ���ڼ����û����ҵ�
        smallFruit = 16, // С����
        largeFruit = 32 // ����
    };

    // �����ID��ȡ��������ҵĶ�����λ
    constexpr GridContentType playerID2Mask[] = { player1, player2, player3, player4 };
    const string playerID2str[] = { "0", "1", "2", "3" };

    // ÿ�����ӹ̶��Ķ���������á����߼��������
    enum GridStaticType
    {
        emptyWall = 0, // ��ʵ�����õ�
        wallNorth = 1, // ��ǽ����������ٵķ���
        wallEast = 2, // ��ǽ�����������ӵķ���
        wallSouth = 4, // ��ǽ�����������ӵķ���
        wallWest = 8, // ��ǽ����������ٵķ���
        generator = 16 // ���Ӳ�����
    };

    // ���ƶ�����ȡ����������赲�ŵ�ǽ�Ķ�����λ
    constexpr GridStaticType direction2OpposingWall[] = { wallNorth, wallEast, wallSouth, wallWest };

    // ���򣬿��Դ���dx��dy���飬ͬʱҲ������Ϊ��ҵĶ���
    enum Direction
    {
        stay = -1,
        up = 0,
        right = 1,
        down = 2,
        left = 3,
        shootUp = 4, // ���Ϸ�����
        shootRight = 5, // ���ҷ�����
        shootDown = 6, // ���·�����
        shootLeft = 7 // ��������
    };

    // �����ϴ�����������
    struct FieldProp
    {
        int row, col;
    };

    // �����ϵ����
    struct Player : FieldProp
    {
        int strength;
        int powerUpLeft;
        bool dead;
    };

    // �غ��²����Ķ��ӵ�����
    struct NewFruits
    {
        FieldProp newFruits[MAX_GENERATOR_COUNT * 8];
        int newFruitCount;
    };
    extern NewFruits newFruits[MAX_TURN];
    extern int newFruitsCount;

    // ״̬ת�Ƽ�¼�ṹ
    struct TurnStateTransfer
    {
        enum StatusChange // �����
        {
            none = 0,
            ateSmall = 1,
            ateLarge = 2,
            powerUpDrop = 4,
            die = 8,
            error = 16
        };

        // ���ѡ���Ķ���
        Direction actions[MAX_PLAYER_COUNT];

        // �˻غϸ���ҵ�״̬�仯
        StatusChange change[MAX_PLAYER_COUNT];

        // �˻غϸ���ҵ������仯
        int strengthDelta[MAX_PLAYER_COUNT];
    };

    // ��Ϸ��Ҫ�߼������࣬��������������غ����㡢״̬ת�ƣ�ȫ��Ψһ
    class GameField
    {
    private:
        // Ϊ�˷��㣬��������Զ�����private��

        // ��������Ƿ��Ѿ�����
        static bool constructed;

    public:
        // ���صĳ��Ϳ�
        int height, width;
        int generatorCount;
        int GENERATOR_INTERVAL, LARGE_FRUIT_DURATION, LARGE_FRUIT_ENHANCEMENT, SKILL_COST;

        // ���ظ��ӹ̶�������
        GridStaticType fieldStatic[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];

        // ���ظ��ӻ�仯������
        GridContentType fieldContent[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];
        int generatorTurnLeft; // ���ٻغϺ��������
        int aliveCount; // �ж�����Ҵ��
        int smallFruitCount;
        int turnID;
        FieldProp generators[MAX_GENERATOR_COUNT]; // ����Щ���Ӳ�����
        Player players[MAX_PLAYER_COUNT]; // ����Щ���

        // ���ѡ���Ķ���
        Direction actions[MAX_PLAYER_COUNT];

        // ��¼ÿ�غϵı仯��ջ��
        TurnStateTransfer backtrack[MAX_TURN];


        // �ָ����ϴγ���״̬������һ·�ָ����ʼ��
        // �ָ�ʧ�ܣ�û��״̬�ɻָ�������false
        bool PopState();

        // �ж�ָ�������ָ�������ƶ�/ʩ�ż����ǲ��ǺϷ��ģ�û��ײǽ��û�вȵ����Ӳ������������㹻��
        bool ActionValid(int playerID, Direction dir) const;

        // ����actionsд����Ҷ�����������һ�غϾ��棬����¼֮ǰ���еĳ���״̬���ɹ��պ�ָ���
        // ���վֵĻ��ͷ���false
        bool NextTurn();

        // ��ȡ�������������룬���ص��Ի��ύƽ̨ʹ�ö����ԡ�
        // ����ڱ��ص��ԣ�����������Ŷ�ȡ������ָ�����ļ���Ϊ�����ļ���ʧ�ܺ���ѡ��ȴ��û�ֱ�����롣
        // ���ص���ʱ���Խ��ܶ����Ա������Windows�¿����� Ctrl-Z ��һ��������+�س�����ʾ���������������������ֻ����ܵ��м��ɡ�
        // localFileName ����ΪNULL
        // obtainedData ������Լ��ϻغϴ洢�����غ�ʹ�õ�����
        // obtainedGlobalData ������Լ��� Bot ����ǰ�洢������
        // ����ֵ���Լ��� playerID
        int ReadInput(const char* localFileName, string& obtainedData, string& obtainedGlobalData);

        // ���� static �� content ����׼�����صĳ�ʼ״��
        void PrepareInitialField(const Json::Value& staticField, const Json::Value& contentField);

        // ��ɾ��ߣ���������
        // action ��ʾ���غϵ��ƶ�����stay Ϊ���ƶ���shoot��ͷ�Ķ�����ʾ��ָ������ʩ�ż���
        // tauntText ��ʾ��Ҫ��������������������ַ�����������ʾ����Ļ�ϲ������κ����ã����ձ�ʾ������
        // data ��ʾ�Լ���洢����һ�غ�ʹ�õ����ݣ����ձ�ʾɾ��
        // globalData ��ʾ�Լ���洢���Ժ�ʹ�õ����ݣ��滻����������ݿ��Կ�Ծ�ʹ�ã���һֱ������� Bot �ϣ����ձ�ʾɾ��
        void WriteOutput(Direction action, string tauntText = "", string data = "", string globalData = "") const;

        // ������ʾ��ǰ��Ϸ״̬�������á�
        // �ύ��ƽ̨��ᱻ�Ż�����
        void DebugPrint() const;

        Json::Value SerializeCurrentTurnChange();

        // ��ʼ����Ϸ������
        GameField();

        //GameField(const GameField &b) : GameField() { }
    };

    extern GameField gameField;
    extern string lastData, globalData;
    extern int myID, turnID;
}
namespace Pacman
{

    void initUtil();
    void parseLastData(const string& lastData);

    extern int distance[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH][FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH]; /* -1��ʾ����ͨ */

    inline int Nmodulo(int x)
    {
        return (x + gameField.height) % gameField.height;
    }
    inline int Mmodulo(int y)
    {
        return (y + gameField.width) % gameField.width;
    }

    void recalc(int& x, GridContentType content, int enhanced);

    struct Point
    {
        int x, y;
        /* x:row y:column */
        Point(int _x = 0, int _y = 0)
            : x(_x), y(_y) {}
    };
    Point operator+ (const Point& a, const Point& b);
    Point operator- (const Point& a, const Point& b);
    bool operator== (const Point& a, const Point& b);
    bool operator< (const Point& a, const Point& b);

    /* ��λ��s�ܷ�ʹ�ý�⳯����d��λ��t */
    bool seeLine(int sr, int sc, int d, int tr, int tc);
    bool seeLine(const Point& s, int d, const Point& t);

    extern Json::Value lastValue, nextValue;

    extern int deadEnd[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];
    extern int maxDeadDepth[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];
    extern int straightDepth[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];

    int cornerFruitSum(int x, int y, int round, bool alreadyUp);
    int transport_quadrant(int q1, int d, int q2);
}

namespace Pacman
{
    struct Danger_Type
    {
        int eaten;
        int hunted;
        int shooted;
        bool dodged;
        bool deadend;
        int beshoot_after_persuit[4];
    };

    extern Danger_Type Isdanger[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];//
    /*
    1:���ܱ�ֱ�ӳԵ�
    2:��һ�����ܽ���
    3:��һ�������д�
    4:�޷��㿪���� ��Ϊ1��ʾ�޷����
    5:����ͬ
    6:hunt�������棬�����»غ��ߵ���λ�ã������˴ӷ��� d ��в���Ƿ���׷����ᱻ��
    1,2,3�ö����Ʊ�ʾ��Щ��ҿ��Զ����λ�������в�������Լ�
    û����һ��λ����û����ң������˸�ǿ��canact����ȥ���������������ù�
    */

    int Isdanger_now();//�ж��Լ������Ƿ�Σ�գ�Σ���򷵻���в�߱�ţ����򷵻�-1,���ÿɲ��õĺ���

    int get_dir(int hx, int hy, int px, int py); //����(px,py)��(hx,hy)��ʲô��λ

    void Init_Isdanger();
    void Init_Isdanger1();
    void Init_Isdanger2();//��ʼ��Isdanger����
    //Ҫдһ���ж�һ��λ�õ�Σ�����͵�ģ��

    struct Escape_result //�Ȳ����ǳ��ֺܴ������ͬ
    {
        int Run_dir;//�Լ�Ӧ���ܵķ���,������-2�����ʾ�����ƣ�ͬʱ��������ģ��
        int Eated;//ֵΪ1��ʾprey�ᱻhunter�Ե����Ƶ�����ͬ���ˣ��򱻼л��ˡ���������ֵΪ2��ʾ���п��ܱ����˳Ե�
        bool Shooted;//prey�Ƿ�һ���ᱻ��
        int Step_to_be_shoot;//һ���ᱻ�򵽵Ļ�����Զ���ܶ��ٲ���0��ʾ��ǰ�غ�
        int beshoot;//һ���ᱻ�򵽵Ļ��������ܴ���

        Escape_result()
        {
            Run_dir = 0;
            Eated = 0;
            Shooted = 0;
            Step_to_be_shoot = 0;
        }
    };

    //��ʱ���Լ������ã��ͼ�Ԥ��׷ɱ���
    Escape_result escape_route(int hunter, int prey);//��дһ������Ԥ����˵����ܷ�������run_dir����-1�����ʾ����

    //�ڲ�����������������£���(px,py)����Է���hun_dir�ĵ�����в�����ܵĽ��
    Escape_result escape_route2(int px, int py, int hun_dir);


    void Escape_main(int hunter);//�Լ������õĺ�������û��˼���
};
#include <set>

namespace Pacman
{
    struct DisInfo
    {
        int dis;
        set<Direction> first_dir;

        DisInfo() { dis = 0; /*first_dir = (Direction) - 1;*/ }
    };

    struct Score
    {
        // TIME    : ʱ��÷֣��������� + ���Ӳ�����ʱ��
        // DANGER  : Σ�յ÷֣������н�·�Ͽ������ܵĹ���
        // STEAL   : ͵���÷֣��������ӱ��������ߵĿ�����
        // DENSITY : �ܶȵ÷֣������ö�����Χ���������ӵĶ���
        // LARGEFRUIT: ���ӵ÷֣�����ö����Ǵ����мӷ֣���������Ժ���Ϸ����ǰ������ʧ�ӷָ���

        const double TIME_RATE = 0.65;
        //const double DANGER_RATE  = 0.0;
        const double STEAL_RATE = 0.0;
        const double DENSITY_RATE = 0.35;
        const double LARGEFRUIT_RATE = 0.3;

        /* ���˸�ƫ��Խ��Ķ��� */
        const double TIME_RATE2 = 0.75;
        //const double DANGER_RATE  = 0.0;
        const double STEAL_RATE2 = 0.0;
        const double DENSITY_RATE2 = 0.25;
        const double LARGEFRUIT_RATE2 = 0.3;

        double tot_score, time_score, /*danger_score, */steal_score, density_score, largefruit_score;

        Score() { tot_score = time_score = steal_score = density_score = largefruit_score = 0; }

        void calc(int id) // -oo ~ 130
        {
            if (id == myID)
                tot_score = time_score * TIME_RATE //+ danger_score * DANGER_RATE
                + steal_score * TIME_RATE + density_score * DENSITY_RATE
                + largefruit_score * LARGEFRUIT_RATE;
            else
                tot_score = time_score * TIME_RATE2 //+ danger_score * DANGER_RATE
                + steal_score * TIME_RATE2 + density_score * DENSITY_RATE2
                + largefruit_score * LARGEFRUIT_RATE2;
        }
    };

    struct Node
    {
        vector<Node*> nxt;
        GridContentType future_type;
        GridContentType content_type;
        GridStaticType static_type;
        DisInfo dis_info;
        Score score;
        int r, c;
        Danger_Type danger;
        bool safe;

        Node()
        {
            future_type = (GridContentType)0;
            content_type = (GridContentType)0;
            static_type = (GridStaticType)0;
            safe = true;
        }
        bool is_now_fruit() { return (content_type & smallFruit) || (content_type & largeFruit); }
        bool is_future_fruit() { return (future_type & smallFruit); }
        bool is_fruit() { return is_now_fruit() || is_future_fruit(); }
    };

    class Eat
    {
    public:
        Node my_map[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];
        GameField* gf;
        vector<FieldProp> small_fruit, large_fruit, fruit, future_fruit;
        map<Node*, int> vis;
        int my_id;
        bool go[4];

        const int dir4[4][2] = { { -1, 0}, {0, 1}, {1, 0}, {0, -1} };
        const int dir8[8][2] = { { -1, 0}, {0, 1}, {1, 0}, {0, -1}, {1, 1}, {1, -1}, { -1, 1}, { -1, -1} };

        const int INF = 1e9;
        const int DENSITY_STEP = 7;

        static FieldProp make_pos(int, int);

    public:
        Eat(int);
        //Direction solve(double &recommend);

        void calc_time();
        //void calc_danger();
        void calc_density();
        void calc_largefruit();

        int dfs(Node*, int, int);
        double get_recommend(int, int);
    };
}
#include <algorithm>
#define V2 8
#define V3 6
#define V4 4
/*
��δ�޸���bug:
1.���˿����ڱ�׷ɱʱ��ͷ���з����������Լ���ͨ��������űȶԷ�ǿ�����ż��ֺ�͹��ˣ����˶��ܱ��ӽ���˵�����ܺ����ϣ�
*/


/*һ���򵥵�׷ɱģ�飬�����Լ����������ʱ��ȥ׷ɱ����
1.�ܷ�ɵ����ˣ��Ƶ�����ͬ���
2.�Ƿ����ҵ����еĻ��ᣬ���ҵ����л���Ļ��������Ҫ����,�����ܷż��У�
3.����һ������ʱ�Ƿ�Ҫ׷ɱ?

��ʾ1: 2*2������ͬ��������ͬ

����ѡ��׷ɱ�Ļ�����һ���ߵ���ǰ�����λ�ü���
*/

namespace Pacman
{
    int killmain(); //������һ��Ӧ��ȡ���ж��������ʺϵ�׷ɱ�ж�������-1
};
#include <set>

namespace Pacman
{

    extern double predict[MAX_PLAYER_COUNT][MAX_ACTION_COUNT];
    struct PredictType
    {
        int hit, sum;
    };
    typedef PredictType PredictTypeList[MAX_PLAYER_COUNT];
    extern PredictTypeList pGetout, pShootGetout, pGiveup;
    extern bool pcanact[MAX_PLAYER_COUNT][MAX_ACTION_COUNT];
    extern bool pesacping[MAX_PLAYER_COUNT];
    typedef bool pActType[MAX_PLAYER_COUNT][MAX_ACTION_COUNT];
    extern pActType pisGetout, pisShootGetout, pisGiveup;

    void initPredict(string& tauntText);

    extern double posexpect[MAX_PLAYER_COUNT][FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];
    extern double pfruitdis[MAX_PLAYER_COUNT][FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];

    void calcpfruitdis(int pl, const set<Point>& ignore);

}
#include <set>

namespace Pacman
{
    set<Point> avoidMain();
    void chaseMain(double pchase[MAX_ACTION_COUNT], string& tauntText);
    void storePhase(GameField* gf, int pl);
    bool matchPhase(GameField* gf, int pl, double ans[MAX_ACTION_COUNT]);
}
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <stdexcept>

namespace Pacman
{
    time_t seed = time(0);

    template<typename T>
    T operator |=(T& a, const T& b)
    {
        return a = static_cast<T>(static_cast<int>(a) | static_cast<int>(b));
    }
    template<typename T>
    T operator |(const T& a, const T& b)
    {
        return static_cast<T>(static_cast<int>(a) | static_cast<int>(b));
    }
    template<typename T>
    T operator &=(T& a, const T& b)
    {
        return a = static_cast<T>(static_cast<int>(a)& static_cast<int>(b));
    }
    template<typename T>
    T operator &(const T& a, const T& b)
    {
        return static_cast<T>(static_cast<int>(a)& static_cast<int>(b));
    }
    template<typename T>
    T operator -(const T& a, const T& b)
    {
        return static_cast<T>(static_cast<int>(a) - static_cast<int>(b));
    }
    template<typename T>
    T operator ++(T& a)
    {
        return a = static_cast<T>(static_cast<int>(a) + 1);
    }
    template<typename T>
    T operator ~(const T& a)
    {
        return static_cast<T>(~static_cast<int>(a));
    }

    NewFruits newFruits[MAX_TURN];
    int newFruitsCount = 0;

    // �ָ����ϴγ���״̬������һ·�ָ����ʼ��
    // �ָ�ʧ�ܣ�û��״̬�ɻָ�������false
    bool GameField::PopState()
    {
        if (turnID <= 0)
            return false;

        const TurnStateTransfer& bt = backtrack[--turnID];
        int i, _;

        // �������ָ�״̬

        for (_ = 0; _ < MAX_PLAYER_COUNT; _++)
        {
            Player& _p = players[_];
            GridContentType& content = fieldContent[_p.row][_p.col];
            TurnStateTransfer::StatusChange change = bt.change[_];

            // 5. �󶹻غϻָ�
            if (change & TurnStateTransfer::powerUpDrop)
                _p.powerUpLeft++;

            // 4. �³�����
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

            // 2. �������
            if (change & TurnStateTransfer::die)
            {
                _p.dead = false;
                aliveCount++;
                content |= playerID2Mask[_];
            }

            // 1. ���λ�Ӱ
            if (!_p.dead && bt.actions[_] != stay && bt.actions[_] < shootUp)
            {
                fieldContent[_p.row][_p.col] &= ~playerID2Mask[_];
                _p.row = (_p.row - dy[bt.actions[_]] + height) % height;
                _p.col = (_p.col - dx[bt.actions[_]] + width) % width;
                fieldContent[_p.row][_p.col] |= playerID2Mask[_];
            }

            // 0. ���겻�Ϸ������
            if (change & TurnStateTransfer::error)
            {
                _p.dead = false;
                aliveCount++;
                content |= playerID2Mask[_];
            }

            // *. �ָ�����
            _p.strength -= bt.strengthDelta[_];
        }

        // 3. �ջض���
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

    // �ж�ָ�������ָ�������ƶ�/ʩ�ż����ǲ��ǺϷ��ģ�û��ײǽ��û�вȵ����Ӳ������������㹻��
    bool GameField::ActionValid(int playerID, Direction dir) const
    {
        if (dir == stay)
            return true;
        const Player& p = players[playerID];
        if (dir >= shootUp)
            return dir < 8 && p.strength > SKILL_COST;
        return dir >= 0 && dir < 4 &&
            !(fieldStatic[p.row][p.col] & direction2OpposingWall[dir]);
    }

    // ����actionsд����Ҷ�����������һ�غϾ��棬����¼֮ǰ���еĳ���״̬���ɹ��պ�ָ���
    // ���վֵĻ��ͷ���false
    bool GameField::NextTurn()
    {
        int _, i, j;

        TurnStateTransfer& bt = backtrack[turnID];
        memset(&bt, 0, sizeof(bt));

        // 0. ɱ�����Ϸ�����
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
                    // �������Լ�ǿ��׳������ǲ���ǰ����
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

        // 1. λ�ñ仯
        for (_ = 0; _ < MAX_PLAYER_COUNT; _++)
        {
            Player& _p = players[_];

            bt.actions[_] = actions[_];

            if (_p.dead || actions[_] == stay || actions[_] >= shootUp)
                continue;

            // �ƶ�
            fieldContent[_p.row][_p.col] &= ~playerID2Mask[_];
            _p.row = (_p.row + dy[actions[_]] + height) % height;
            _p.col = (_p.col + dx[actions[_]] + width) % width;
            fieldContent[_p.row][_p.col] |= playerID2Mask[_];
        }

        // 2. ��һ�Ź
        for (_ = 0; _ < MAX_PLAYER_COUNT; _++)
        {
            Player& _p = players[_];
            if (_p.dead)
                continue;

            // �ж��Ƿ��������һ��
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

                // ��Щ��ҽ��ᱻɱ��
                int lootedStrength = 0;
                for (i = begin; i < containedCount; i++)
                {
                    int id = containedPlayers[i];
                    Player& p = players[id];

                    // �Ӹ���������
                    fieldContent[p.row][p.col] &= ~playerID2Mask[id];
                    p.dead = true;
                    int drop = p.strength / 2;
                    bt.strengthDelta[id] += -drop;
                    bt.change[id] |= TurnStateTransfer::die;
                    lootedStrength += drop;
                    p.strength -= drop;
                    aliveCount--;
                }

                // ������������
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

        // 2.5 ��ⷨ��
        for (_ = 0; _ < MAX_PLAYER_COUNT; _++)
        {
            Player& _p = players[_];
            if (_p.dead || actions[_] < shootUp)
                continue;

            _p.strength -= SKILL_COST;
            bt.strengthDelta[_] -= SKILL_COST;

            int r = _p.row, c = _p.col, player;
            Direction dir = actions[_] - shootUp;

            // ��ָ���������⣨ɨ�����ֱ��������
            while (!(fieldStatic[r][c] & direction2OpposingWall[dir]))
            {
                r = (r + dy[dir] + height) % height;
                c = (c + dx[dir] + width) % width;

                // ���ת��һȦ��������
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

        // *. ���һ�������������
        for (_ = 0; _ < MAX_PLAYER_COUNT; _++)
        {
            Player& _p = players[_];
            if (_p.dead || _p.strength > 0)
                continue;

            // �Ӹ���������
            fieldContent[_p.row][_p.col] &= ~playerID2Mask[_];
            _p.dead = true;

            // ʹ��������Ϊ0
            bt.strengthDelta[_] += -_p.strength;
            bt.change[_] |= TurnStateTransfer::die;
            _p.strength = 0;
            aliveCount--;
        }


        // 3. ��������
        if (--generatorTurnLeft == 0)
        {
            generatorTurnLeft = GENERATOR_INTERVAL;
            NewFruits& fruits = newFruits[newFruitsCount++];
            fruits.newFruitCount = 0;
            for (i = 0; i < generatorCount; i++)
                for (Direction d = up; d < 8; ++d)
                {
                    // ȡ�࣬�������ر߽�
                    int r = (generators[i].row + dy[d] + height) % height, c = (generators[i].col + dx[d] + width) % width;
                    if (fieldStatic[r][c] & generator || fieldContent[r][c] & (smallFruit | largeFruit))
                        continue;
                    fieldContent[r][c] |= smallFruit;
                    fruits.newFruits[fruits.newFruitCount].row = r;
                    fruits.newFruits[fruits.newFruitCount++].col = c;
                    smallFruitCount++;
                }
        }

        // 4. �Ե�����
        for (_ = 0; _ < MAX_PLAYER_COUNT; _++)
        {
            Player& _p = players[_];
            if (_p.dead)
                continue;

            GridContentType& content = fieldContent[_p.row][_p.col];

            // ֻ���ڸ�����ֻ���Լ���ʱ����ܳԵ�����
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

        // 5. �󶹻غϼ���
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

        // *. ���һ�������������
        for (_ = 0; _ < MAX_PLAYER_COUNT; _++)
        {
            Player& _p = players[_];
            if (_p.dead || _p.strength > 0)
                continue;

            // �Ӹ���������
            fieldContent[_p.row][_p.col] &= ~playerID2Mask[_];
            _p.dead = true;

            // ʹ��������Ϊ0
            bt.strengthDelta[_] += -_p.strength;
            bt.change[_] |= TurnStateTransfer::die;
            _p.strength = 0;
            aliveCount--;
        }

        ++turnID;

        // �Ƿ�ֻʣһ�ˣ�
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

        // �Ƿ�غϳ��ޣ�
        if (turnID >= 100)
            return false;

        return true;
    }

    // ��ȡ�������������룬���ص��Ի��ύƽ̨ʹ�ö����ԡ�
    // ����ڱ��ص��ԣ�����������Ŷ�ȡ������ָ�����ļ���Ϊ�����ļ���ʧ�ܺ���ѡ��ȴ��û�ֱ�����롣
    // ���ص���ʱ���Խ��ܶ����Ա������Windows�¿����� Ctrl-Z ��һ��������+�س�����ʾ���������������������ֻ����ܵ��м��ɡ�
    // localFileName ����ΪNULL
    // obtainedData ������Լ��ϻغϴ洢�����غ�ʹ�õ�����
    // obtainedGlobalData ������Լ��� Bot ����ǰ�洢������
    // ����ֵ���Լ��� playerID
    int GameField::ReadInput(const char* localFileName, string& obtainedData, string& obtainedGlobalData)
    {
        string str, chunk;
#ifdef _BOTZONE_ONLINE
        std::ios::sync_with_stdio(false); //��\\)
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

        // ��ȡ���ؾ�̬״��
        Json::Value field = input["requests"][(Json::Value::UInt) 0],
            staticField = field["static"], // ǽ��Ͳ�����
            contentField = field["content"]; // ���Ӻ����
        height = field["height"].asInt();
        width = field["width"].asInt();
        LARGE_FRUIT_DURATION = field["LARGE_FRUIT_DURATION"].asInt();
        LARGE_FRUIT_ENHANCEMENT = field["LARGE_FRUIT_ENHANCEMENT"].asInt();
        SKILL_COST = field["SKILL_COST"].asInt();
        generatorTurnLeft = GENERATOR_INTERVAL = field["GENERATOR_INTERVAL"].asInt();

        PrepareInitialField(staticField, contentField);

        initUtil();

        // ������ʷ�ָ�����
        for (int i = 1; i < len; i++)
        {
            Json::Value req = input["requests"][i];
            for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
                if (!players[_].dead)
                    actions[_] = (Direction)req[playerID2str[_]]["action"].asInt();

            // ����phase
            for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
                if (!players[_].dead)
                    storePhase(this, _);

            NextTurn();
        }

        Pacman::turnID = this->turnID;

        obtainedData = input["data"].asString();
        obtainedGlobalData = input["globaldata"].asString();

        return field["id"].asInt();
    }

    // ���� static �� content ����׼�����صĳ�ʼ״��
    void GameField::PrepareInitialField(const Json::Value& staticField, const Json::Value& contentField)
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

    // ��ɾ��ߣ���������
    // action ��ʾ���غϵ��ƶ�����stay Ϊ���ƶ���shoot��ͷ�Ķ�����ʾ��ָ������ʩ�ż���
    // tauntText ��ʾ��Ҫ��������������������ַ�����������ʾ����Ļ�ϲ������κ����ã����ձ�ʾ������
    // data ��ʾ�Լ���洢����һ�غ�ʹ�õ����ݣ����ձ�ʾɾ��
    // globalData ��ʾ�Լ���洢���Ժ�ʹ�õ����ݣ��滻����������ݿ��Կ�Ծ�ʹ�ã���һֱ������� Bot �ϣ����ձ�ʾɾ��
    void GameField::WriteOutput(Direction action, string tauntText, string data, string globalData) const
    {
        Json::Value ret;
        ret["response"]["action"] = action;
        ret["response"]["tauntText"] = tauntText;
        ret["data"] = data;
        ret["globaldata"] = globalData;
        ret["debug"] = (Json::Int)seed;

#ifdef _BOTZONE_ONLINE
        Json::FastWriter writer; // ��������Ļ����þ��С���
#else
        Json::StyledWriter writer; // ���ص��������ÿ� > <
#endif
        cout << writer.write(ret) << endl;
    }

    // ������ʾ��ǰ��Ϸ״̬�������á�
    // �ύ��ƽ̨��ᱻ�Ż�����
    void GameField::DebugPrint() const
    {
#ifndef _BOTZONE_ONLINE
        printf("�غϺš�%d�����������%d��| ͼ�� ������[G] �����[0/1/2/3] ������[*] ��[o] С��[.]\n", turnID, aliveCount);
        for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
        {
            const Player& p = players[_];
            printf("[���%d(%d, %d)|����%d|�ӳ�ʣ��غ�%d|%s]\n",
                _, p.row, p.col, p.strength, p.powerUpLeft, p.dead ? "����" : "���");
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
                    {
                        if (hasPlayer == -1)
                            hasPlayer = _;
                        else
                            hasPlayer = 4;
                    }
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

    Json::Value GameField::SerializeCurrentTurnChange()
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

    // ��ʼ����Ϸ������
    GameField::GameField()
    {
        if (constructed)
            throw runtime_error("please don't construct GameField object ");
        //throw runtime_error("�벻Ҫ�ٴ��� GameField �����ˣ�����������ֻӦ����һ������");

        constructed = true;

        turnID = 0;
    }

    bool GameField::constructed = false;

    GameField gameField;
    string lastData, globalData;
    int myID, turnID;
}
#include <queue>
#include <cstring>

namespace Pacman
{

    int distance[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH][FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH]; /* -1��ʾ����ͨ */

    void calcDistance(int sx, int sy)
    {
        int(*dis)[FIELD_MAX_WIDTH] = distance[sx][sy];
        memset(dis, 0xff, sizeof(int) * FIELD_MAX_HEIGHT * FIELD_MAX_WIDTH);
        dis[sx][sy] = 0;
        struct Tque
        {
            int x, y, d;
            Tque(int _x, int _y, int _d)
                : x(_x), y(_y), d(_d) {}
        };
        queue<Tque> q;
        q.push(Tque(sx, sy, 0));
        while (!q.empty())
        {
            Tque fr = q.front();
            q.pop();
            for (int i = 0; i < 4; ++i)
                if ((gameField.fieldStatic[fr.x][fr.y] & direction2OpposingWall[i]) == 0)
                {
                    int tx = Nmodulo(fr.x + dir[i][0]), ty = Mmodulo(fr.y + dir[i][1]);
                    if (dis[tx][ty] == -1)
                    {
                        dis[tx][ty] = fr.d + 1;
                        q.push(Tque(tx, ty, fr.d + 1));
                    }
                }
        }
    }
    void initDistance()
    {
        for (int i = 0; i < gameField.height; ++i)
            for (int j = 0; j < gameField.width; ++j)
                calcDistance(i, j);
    }

    Point operator+ (const Point& a, const Point& b)
    {
        return Point(a.x + b.x, a.y + b.y);
    }
    Point operator- (const Point& a, const Point& b)
    {
        return Point(a.x - b.x, a.y - b.y);
    }
    bool operator== (const Point& a, const Point& b)
    {
        return a.x == b.x && a.y == b.y;
    }
    bool operator< (const Point& a, const Point& b)
    {
        return a.x < b.x || (a.x == b.x && a.y < b.y);
    }

    bool seeLine(int sr, int sc, int d, int tr, int tc)
    {
        int x = sr, y = sc;
        while ((gameField.fieldStatic[x][y] & direction2OpposingWall[d]) == 0)
        {
            x = Nmodulo(x + dir[d][0]);
            y = Mmodulo(y + dir[d][1]);
            if (x == sr && y == sc)
                break;
            if (x == tr && y == tc)
                return true;
        }
        return false;
    }

    bool seeLine(const Point& s, int d, const Point& t)
    {
        return seeLine(s.x, s.y, d, t.x, t.y);
    }

    void recalc(int& x, GridContentType content, int enhanced)
    {
        if (enhanced == 1)
            x -= 10;
        if (content & smallFruit)
            x += 1;
        if (content & largeFruit)
            if (enhanced <= 1)
                x += 10;
    }

    Json::Value lastValue, nextValue;
    void parseLastData(const string& lastData)
    {
        Json::Reader reader;
        reader.parse(lastData, lastValue);
    }

    int maxDeadDepth[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];
    int deadEnd[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];
    bool checkDeadEnd(int x, int y)
    {
        if (deadEnd[x][y])
            return true;
        int cnt = 0;
        for (int i = 0; i < 4; ++i)
        {
            if (gameField.fieldStatic[x][y] & direction2OpposingWall[i])
                ++cnt;
            else if (deadEnd[Nmodulo(x + dir[i][0])][Mmodulo(y + dir[i][1])])
                ++cnt;
        }
        deadEnd[x][y] = cnt >= 3;
        return deadEnd[x][y];
    }
    void dfsDeadEnd(int x, int y)
    {
        static bool vis[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH] = {};
        if (vis[x][y])
            return;
        vis[x][y] = true;
        for (int i = 0; i < 4; ++i)
            if ((gameField.fieldStatic[x][y] & direction2OpposingWall[i]) == 0)
            {
                int tx = Nmodulo(x + dir[i][0]);
                int ty = Mmodulo(y + dir[i][1]);
                if (deadEnd[tx][ty] && !vis[tx][ty])
                    deadEnd[tx][ty] = deadEnd[x][y] + 1;
                dfsDeadEnd(tx, ty);
            }
    }
    int dfsDeadDepth(int x, int y)
    {
        int ans = deadEnd[x][y];
        for (int i = 0; i < 4; ++i)
            if ((gameField.fieldStatic[x][y] & direction2OpposingWall[i]) == 0)
            {
                int tx = Nmodulo(x + dir[i][0]);
                int ty = Mmodulo(y + dir[i][1]);
                if (deadEnd[tx][ty] > deadEnd[x][y])
                    ans = max(ans, dfsDeadDepth(tx, ty));
            }
        return ans;
    }
    void initDeadEnd()
    {
        memset(deadEnd, 0, sizeof(deadEnd));
        queue<Point> q;
        for (int i = 0; i < gameField.height; ++i)
            for (int j = 0; j < gameField.width; ++j)
                if (checkDeadEnd(i, j))
                    q.push(Point(i, j));
        while (!q.empty())
        {
            Point fr = q.front();
            q.pop();
            for (int i = 0; i < 4; ++i)
                if ((gameField.fieldStatic[fr.x][fr.y] & direction2OpposingWall[i]) == 0)
                {
                    int tx = Nmodulo(fr.x + dir[i][0]), ty = Mmodulo(fr.y + dir[i][1]);
                    if (!deadEnd[tx][ty] && checkDeadEnd(tx, ty))
                        q.push(Point(tx, ty));
                }
        }

        for (int i = 0; i < gameField.height; ++i)
            for (int j = 0; j < gameField.width; ++j)
                if (!deadEnd[i][j])
                {
                    dfsDeadEnd(i, j);
                    break;
                }
        for (int i = 0; i < gameField.height; ++i)
            for (int j = 0; j < gameField.width; ++j)
                if (deadEnd[i][j])
                    maxDeadDepth[i][j] = dfsDeadDepth(i, j) - deadEnd[i][j] + 1;
    }

    int straightDepth[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];
    int calcStraightDepth(int sx, int sy)
    {
        int ans = 100;
        for (int i = 0; i < gameField.height; ++i)
            for (int j = 0; j < gameField.width; ++j)
                if (straightDepth[i][j] == 0)
                    ans = min(ans, distance[sx][sy][i][j]);
        if (ans == -1)
            ans = 1;
        return ans;
    }

    void initStraightDepth()
    {
        for (int i = 0; i < gameField.height; ++i)
            for (int j = 0; j < gameField.width; ++j)
            {
                if ((gameField.fieldStatic[i][j] & (1 | 4)) == (1 | 4))
                    straightDepth[i][j] = 1;
                else if ((gameField.fieldStatic[i][j] & (2 | 8)) == (2 | 8))
                    straightDepth[i][j] = 1;
                else
                    straightDepth[i][j] = 0;
            }
        for (int i = 0; i < gameField.height; ++i)
            for (int j = 0; j < gameField.width; ++j)
                if (straightDepth[i][j])
                    straightDepth[i][j] = calcStraightDepth(i, j);
    }

    void initUtil()
    {
        initDistance();
        initDeadEnd();
        initStraightDepth();
    }

    int cornerFruitSum(int x, int y, int round, bool alreadyUp)
    {
        bool vis[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH] = {};
        queue<Point> q;
        q.push(Point(x, y));
        int ans = 0;
        vis[x][y] = true;
        while (!q.empty())
        {
            Point fr = q.front();
            q.pop();
            if (gameField.fieldContent[fr.x][fr.y] & smallFruit)
                ans++;
            if ((gameField.fieldContent[fr.x][fr.y] & largeFruit) && !alreadyUp)
            {
                ans += 10;
                alreadyUp = true;
            }
            for (int i = 0; i < 4; ++i)
            {
                if ((gameField.fieldStatic[fr.x][fr.y] & direction2OpposingWall[i]) != 0)
                    continue;
                int tx = Nmodulo(fr.x + dir[i][0]);
                int ty = Mmodulo(fr.y + dir[i][1]);
                if (gameField.generatorTurnLeft <= round && (gameField.fieldContent[tx][ty] & (smallFruit | largeFruit)) == 0)
                    for (int g = 0; g < gameField.generatorCount; ++g)
                        for (int d = 0; d < 8; ++d)
                            if (tx == Nmodulo(gameField.generators[g].row + dy[d]) && ty == Mmodulo(gameField.generators[g].col + dx[d]))
                            {
                                ++ans;
                                goto jumpout;
                            }
            jumpout:
                if (!vis[tx][ty] && deadEnd[tx][ty])
                {
                    q.push(Point(tx, ty));
                    vis[tx][ty] = true;
                }
            }
        }
        return ans;
    }

    /*
     * 0|1
     * ---
     * 2|3
     */
    int transport_quadrant(int q1, int d, int q2)
    {
        if (q1 == q2 || d == -1)
            return d;
        if (d >= shootUp)
            return transport_quadrant(q1, d - shootUp, q2) + shootUp;
        if (q1 == (q2 ^ 3))
            return d ^ 2;
        if (q1 == (q2 ^ 1))
            return d == 0 || d == 2 ? d : (d ^ 2);
        // if (q1 == (q2 ^ 2))
        return d == 1 || d == 3 ? d : (d ^ 2);
    }
}
#include <cstring>
#include <utility>
#include <algorithm>
#include <iostream>
#define LONGEST_DFS_DEP 15
#define debug(x) cerr<<#x<<"="<<x<<endl
//#define DEBUG2

typedef std::pair<int, int> pii;
namespace Pacman
{
    int Isdanger_now()
    {
        int i, sx, sy, nx, ny, flag = -1;
        sx = gameField.players[myID].row;
        sy = gameField.players[myID].col;
        for (i = 0; i < MAX_PLAYER_COUNT; i++)
            if (i != myID && !gameField.players[i].dead)
            {
                nx = gameField.players[i].row;
                ny = gameField.players[i].col;
                if (distance[sx][sy][nx][ny] == 1 && gameField.players[i].strength > gameField.players[myID].strength)
                    flag = i;
            }
        return flag;
    }

    int get_dir(int hx, int hy, int px, int py)
    {
        for (int d = 0; d < 4; d++)
            if (Nmodulo(hx + dir[d][0]) == px && Mmodulo(hy + dir[d][1]) == py)
                return d;
    }

    Danger_Type Isdanger[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];

    void Init_Isdanger()
    {
        Init_Isdanger1();
        Init_Isdanger2();
    }

    void Init_Isdanger1()
    {
        memset(Isdanger, 0, sizeof(Isdanger));
        int x, y, i, nx, ny, d;
        for (x = 0; x < gameField.height; x++)
            for (y = 0; y < gameField.width; y++)
            {
                for (i = 0; i < MAX_PLAYER_COUNT; i++)
                {
                    if (gameField.players[i].dead)
                        continue;
                    nx = gameField.players[i].row;
                    ny = gameField.players[i].col;
                    if (distance[x][y][nx][ny] == 1)
                        Isdanger[x][y].eaten |= 1 << i;
                    else if (distance[x][y][nx][ny] == 2)
                        Isdanger[x][y].hunted |= 1 << i;
                    for (d = 0; d < 4; d++)
                    {
                        if (seeLine(nx, ny, d, x, y) && gameField.players[i].strength > gameField.SKILL_COST)
                            Isdanger[x][y].shooted |= 1 << i;
                    }
                }

                if ((gameField.fieldStatic[x][y] & direction2OpposingWall[0]) && (gameField.fieldStatic[x][y] & direction2OpposingWall[2]))
                    Isdanger[x][y].dodged = 1;

                if ((gameField.fieldStatic[x][y] & direction2OpposingWall[1]) && (gameField.fieldStatic[x][y] & direction2OpposingWall[3]))
                    Isdanger[x][y].dodged = 1;

                Isdanger[x][y].deadend = deadEnd[x][y];
            }
    }

    bool escape_vis[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];
    int fruit_sum;
    void countfruit_dfs(int x, int y)
    {
        int d, nx, ny;
        escape_vis[x][y] = 1;
        if ((gameField.fieldContent[x][y] & smallFruit))
            fruit_sum++;
        if ((gameField.fieldContent[x][y] & largeFruit))
            fruit_sum += 10;

        for (d = 0; d < 4; d++)
            if ((gameField.fieldStatic[x][y] & direction2OpposingWall[d]) == 0)
            {
                nx = Nmodulo(x + dir[d][0]);
                ny = Mmodulo(y + dir[d][1]);
                if (!escape_vis[nx][ny])
                    countfruit_dfs(nx, ny);
            }
    }

    int got_shoot_calc(int px, int py, int d)//��������λ�ÿ�ʼ�ܣ��ᱻ����
    {
        int cnt = 0;
        while (Isdanger[px][py].dodged)
        {
            cnt++;
            if ((gameField.fieldStatic[px][py] & direction2OpposingWall[d]) != 0)
                continue;
            //debug(cnt);
            px = Nmodulo(px + dir[d][0]);
            py = Mmodulo(py + dir[d][1]);
        }
        return cnt;
    }

    pii escape_dfs(int px, int py, int last_dir, int dep)
    {
#ifdef DEBUG
        debug(px);
        debug(py);
#endif // DEBUG
        int best_dir, min_shoot = 100, max_step, nx, ny, d;
        pii tmp;
        escape_vis[px][py] = 1;
        if (dep > LONGEST_DFS_DEP)//���˺�Զ���͵��ܵ���
            return make_pair(0, 0);

        if (Isdanger[px][py].dodged)//�ߵ��˻ᱻ��ĵط�
            return make_pair(got_shoot_calc(px, py, last_dir), 1);

#ifdef DEBUG
        debug(px);
        debug(py);
#endif // DEBUG

        for (d = 0; d < 4; d++)
            if ((d != (last_dir ^ 2)) && (gameField.fieldStatic[px][py] & direction2OpposingWall[d]) == 0)
            {
                nx = Nmodulo(px + dir[d][0]);
                ny = Mmodulo(py + dir[d][1]);
#ifdef DEBUG
                debug(nx);
                debug(ny);
#endif // DEBUG
                if (!escape_vis[nx][ny])
                {
                    if (Isdanger[nx][ny].deadend)//��������ͬ����
                        continue;
                    tmp = escape_dfs(nx, ny, d, dep + 1);
                    if (tmp.first < min_shoot)
                        min_shoot = tmp.first, max_step = tmp.second + 1;
                    else if (tmp.first == min_shoot)
                        max_step = max(max_step, tmp.second + 1);
                }
                else
                {
                    return make_pair(0, 0);
                }//�ҵ�·��
            }
        return make_pair(min_shoot, max_step);
    }

    Escape_result escape_route(int hunter, int prey)
    {
        Escape_result rel;
        int notgo_dir[4];//���һЩ���򣬲�ָ���������ԭ��1.ײǽ������ǿ������ײ 2.������ͬ����
        int res_dir = 4;//��ʣ���ٸ�����û�����
        memset(notgo_dir, 0, sizeof(notgo_dir));
        int hx = gameField.players[hunter].row, hy = gameField.players[hunter].col;
        int px = gameField.players[prey].row, py = gameField.players[prey].col;
        int nx, ny, d, k;

        //����Ѿ�������ͬ����ü��ӣ������ˡ�����û����2*2������ͬ
        if (Isdanger[px][py].deadend)
        {
            rel.Eated = 1;
            rel.Run_dir = -2;
            return rel;
        }

        //�ȷ����ǿ��ײ�ķ������ײǽ
        for (d = 0; d < 4; d++)
        {
            if ((gameField.fieldStatic[px][py] & direction2OpposingWall[d]) == 0)
            {
                nx = Nmodulo(px + dir[d][0]);
                ny = Mmodulo(py + dir[d][1]);

                bool hvstrong = false;
                for (k = 0; k < MAX_PLAYER_COUNT; ++k)
                    if (!gameField.players[k].dead && gameField.players[k].strength > gameField.players[prey].strength
                        && gameField.players[k].row == nx && gameField.players[k].col == ny)
                        hvstrong = true;
                if (hvstrong)
                    notgo_dir[d] = 1, res_dir--;
            }
            else notgo_dir[d] = 1, res_dir--;
        }
        if (res_dir == 0)//�Ѿ����Ƶ����Ľ������ˣ�������
        {
            rel.Eated = 1;
            rel.Run_dir = get_dir(px, py, hx, hy);
            return rel;
        }
        //��������ʼ�ų����Ⱦ�����һ��
        //debug("haven't been sixunhuan0");
        //�����Ƿ�һ�����ߵ�����ͬ�����������ߵ�����ͬ��ʵ�ڲ��������涹����������ͬ��ȥ��ͬʱ���Ƽ���
        int best_deadend = -1, max_fruit = -1;
        for (d = 0; d < 4; d++)
            if (!notgo_dir[d])
            {
                nx = Nmodulo(px + dir[d][0]);
                ny = Mmodulo(py + dir[d][1]);
                if (Isdanger[nx][ny].deadend)
                {
                    notgo_dir[d] = 2;
                    res_dir--;
                    //ͳ���������ͬ���ж��ٶ��ӣ���ʱ���涹��Ҳ�㣬����������ͬ�ﻹ��������ҡ�����
                    memset(escape_vis, 0, sizeof(escape_vis));
                    escape_vis[px][py] = 1;
                    fruit_sum = 0;
                    countfruit_dfs(nx, ny);
                    if (max_fruit < fruit_sum)
                        max_fruit = fruit_sum, best_deadend = d;
                }
            }
        //ֻ��������ͬ�����ˣ�ѡ����õ�
        if (res_dir == 0)
        {
            rel.Eated = 1;
            rel.Run_dir = best_deadend;
            return rel;
        }
        //debug("haven't been sixunhuan1");
        //Ȼ������Ҫ���Լ��͵�������ߣ����˿����������һ��������
        for (d = 0; d < 4; d++)
            if (!notgo_dir[d])
            {
                nx = Nmodulo(px + dir[d][0]);
                ny = Mmodulo(py + dir[d][1]);
                for (k = 0; k < MAX_PLAYER_COUNT; ++k)
                    if ((Isdanger[nx][ny].eaten & (1 << k)) && gameField.players[k].strength > gameField.players[prey].strength)
                    {
                        notgo_dir[d] = 3;
                        res_dir--;
                    }
            }
        if (res_dir == 0)//����·�����ˣ������һ���˳Ե����ˡ�����
        {
            for (d = 0; d < 4; d++)
                if (notgo_dir[d] == 3)
                {
                    rel.Eated = 2;
                    rel.Run_dir = d;
                    rel.Shooted = 0;
                    return rel;
                }
        }
        //������һ�����Ѿ���ֻ��һ����������ܣ��Ҵ���һ���ᱻ�򵽵�����λ��
        if (res_dir == 1 && Isdanger[px][py].dodged)
        {
            rel.Eated = 0;
            rel.Shooted = 1;
            rel.Step_to_be_shoot = 0;//����Ӧ�þͻᱻ�ɡ�����
            for (d = 0; d < 4; d++)
                if (!notgo_dir[d])
                {
                    rel.Run_dir = d;
                    break;
                }
            rel.beshoot = got_shoot_calc(px, py, rel.Run_dir);

            if (gameField.players[hunter].strength > gameField.SKILL_COST&& gameField.players[hunter].strength <= gameField.SKILL_COST * 2) //�Է�ֻ�ܴ�һ��������������
                if (gameField.players[prey].strength > gameField.SKILL_COST)
                    rel.Run_dir = (d ^ 2) + 4;//�������

            if (rel.Run_dir < shootUp && rel.beshoot * gameField.SKILL_COST * 1.5 >= gameField.players[prey].strength)//�ᱻ���������練�������
            {
                if (gameField.players[prey].strength > gameField.SKILL_COST)
                    rel.Run_dir = (d ^ 2) + 4;//�������
                else;//��Ӧ�õ�����rel.Run_dir = -1;//ԭ�ص���
            }
            return rel;
        }
        //debug("haven't been sixunhuan2");

        //Ӧ�ð�danger��������������
        int best_dir, min_shoot = 100, max_step, tmp, may_be_shoot, danger;
        pii tmp2;
        for (d = 0; d < 4; d++)
            if (!notgo_dir[d])
            {
                nx = Nmodulo(px + dir[d][0]);
                ny = Mmodulo(py + dir[d][1]);
                danger = 0;
                for (int i = 0; i < MAX_PLAYER_COUNT; i++)
                    if (Isdanger[px][py].shooted & (1 << i))
                        danger = 1;

                memset(escape_vis, 0, sizeof(escape_vis));
                escape_vis[px][py] = 1;
                tmp2 = escape_dfs(nx, ny, d, 1);

                if (tmp2.first < min_shoot)
                {
                    min_shoot = tmp2.first, max_step = tmp2.second + 1;
                    may_be_shoot = danger;
                    best_dir = d;
                }
                else if (tmp2.first == min_shoot)
                {
                    if (danger < may_be_shoot)
                    {
                        min_shoot = tmp2.first, max_step = tmp2.second + 1;
                        may_be_shoot = danger;
                        best_dir = d;
                    }
                    else if (danger == may_be_shoot && max_step < tmp2.second + 1)
                    {
                        min_shoot = tmp2.first, max_step = tmp2.second + 1;
                        may_be_shoot = danger;
                        best_dir = d;
                    }
                }
            }

        rel.Eated = 0;
        rel.Run_dir = best_dir;
        if (min_shoot == 0)
            rel.Shooted = 0;
        else rel.Shooted = 1;
        rel.beshoot = min_shoot;
        rel.Step_to_be_shoot = max_step;

        if (rel.beshoot * gameField.SKILL_COST * 1.5 >= gameField.players[prey].strength && rel.Step_to_be_shoot == 1)//�ᱻ������Ҳ�ϵ����һ���ˣ��������
            rel.Run_dir = -1;//ԭ�ص���
        return rel;
    }

    Escape_result escape_route2(int px, int py, int hun_dir)//�����������px,py��hun_dir����
    {
        Escape_result rel;
        int notgo_dir[4];//���һЩ���򣬲�ָ���������ԭ��1.ײǽ������ǿ������ײ 2.������ͬ����
        int res_dir = 4;//��ʣ���ٸ�����û�����
        memset(notgo_dir, 0, sizeof(notgo_dir));
        int hx = Nmodulo(px + dir[hun_dir][0]), hy = Mmodulo(py + dir[hun_dir][1]);
        int nx, ny, d, k;

        //����Ѿ�������ͬ����ü��ӣ������ˡ�����û����2*2������ͬ
        if (Isdanger[px][py].deadend)
        {
            rel.Eated = 1;
            rel.Run_dir = -2;
            return rel;
        }

        //�ȷ����ǿ��ײ�ķ������ײǽ
        for (d = 0; d < 4; d++)
        {
            if ((gameField.fieldStatic[px][py] & direction2OpposingWall[d]) == 0)
            {
                if (d == hun_dir)
                    notgo_dir[d] = 1, res_dir--;
            }
            else notgo_dir[d] = 1, res_dir--;
        }
        if (res_dir == 0)//�Ѿ����Ƶ����Ľ������ˣ�������
        {
            rel.Eated = 1;
            rel.Run_dir = get_dir(px, py, hx, hy);
            return rel;
        }
        //��������ʼ�ų����Ⱦ�����һ��
        //debug("haven't been sixunhuan0");
        //�����Ƿ�һ�����ߵ�����ͬ�����������ߵ�����ͬ��ʵ�ڲ��������涹����������ͬ��ȥ��ͬʱ���Ƽ���
        int best_deadend = -1, max_fruit = -1;
        for (d = 0; d < 4; d++)
            if (!notgo_dir[d])
            {
                nx = Nmodulo(px + dir[d][0]);
                ny = Mmodulo(py + dir[d][1]);
                if (Isdanger[nx][ny].deadend)
                {
                    notgo_dir[d] = 2;
                    res_dir--;
                    //ͳ���������ͬ���ж��ٶ��ӣ���ʱ���涹��Ҳ�㣬����������ͬ�ﻹ��������ҡ�����
                    memset(escape_vis, 0, sizeof(escape_vis));
                    escape_vis[px][py] = 1;
                    fruit_sum = 0;
                    countfruit_dfs(nx, ny);
                    if (max_fruit < fruit_sum)
                        max_fruit = fruit_sum, best_deadend = d;
                }
            }
        //ֻ��������ͬ�����ˣ�ѡ����õ�
        if (res_dir == 0)
        {
            rel.Eated = 1;
            rel.Run_dir = best_deadend;
            return rel;
        }
        //debug("haven't been sixunhuan1");
        //delete1

        //������һ�����Ѿ���ֻ��һ����������ܣ��Ҵ���һ���ᱻ�򵽵�����λ��
        if (res_dir == 1 && Isdanger[px][py].dodged)
        {
            rel.Eated = 0;
            rel.Shooted = 1;
            rel.Step_to_be_shoot = 0;//����Ӧ�þͻᱻ�ɡ�����
            for (d = 0; d < 4; d++)
                if (!notgo_dir[d])
                    rel.Run_dir = d;
            rel.beshoot = got_shoot_calc(px, py, rel.Run_dir);
            //delete2
            return rel;
        }
        //debug("haven't been sixunhuan2");

        //Ӧ�ð�danger��������������
        int best_dir, min_shoot = 100, max_step, tmp, may_be_shoot, danger;
        pii tmp2;
        for (d = 0; d < 4; d++)
            if (!notgo_dir[d])
            {
                nx = Nmodulo(px + dir[d][0]);
                ny = Mmodulo(py + dir[d][1]);
                danger = 0;
                for (int i = 0; i < MAX_PLAYER_COUNT; i++)
                    if (Isdanger[px][py].shooted & (1 << i))
                        danger = 1;

                memset(escape_vis, 0, sizeof(escape_vis));
                escape_vis[px][py] = 1;
                tmp2 = escape_dfs(nx, ny, d, 1);

                if (tmp2.first < min_shoot)
                {
                    min_shoot = tmp2.first, max_step = tmp2.second + 1;
                    may_be_shoot = danger;
                    best_dir = d;
                }
                else if (tmp2.first == min_shoot)
                {
                    if (danger < may_be_shoot)
                    {
                        min_shoot = tmp2.first, max_step = tmp2.second + 1;
                        may_be_shoot = danger;
                        best_dir = d;
                    }
                    else if (danger == may_be_shoot && max_step < tmp2.second + 1)
                    {
                        min_shoot = tmp2.first, max_step = tmp2.second + 1;
                        may_be_shoot = danger;
                        best_dir = d;
                    }
                }
            }

        rel.Eated = 0;
        rel.Run_dir = best_dir;
        if (min_shoot == 0)
            rel.Shooted = 0;
        else rel.Shooted = 1;
        rel.beshoot = min_shoot;
        rel.Step_to_be_shoot = max_step;

        //if (rel.beshoot * 9 >= gameField.players[prey].strength && rel.Step_to_be_shoot == 1)//�ᱻ������Ҳ�ϵ����һ���ˣ��������
            //rel.Run_dir = -1;//ԭ�ص���
        return rel;
    }

    void Init_Isdanger2()
    {
        //memset(Isdanger, 0, sizeof(Isdanger));
        int x, y, i, nx, ny, d;
        Escape_result rel;
        for (x = 0; x < gameField.height; x++)
            for (y = 0; y < gameField.width; y++)
                for (d = 0; d < 4; d++)
                {
                    if (gameField.fieldStatic[x][y] & direction2OpposingWall[d])
                    {
                        Isdanger[x][y].beshoot_after_persuit[d] = 0;
                        continue;
                    }
                    rel = escape_route2(x, y, d);
                    if (rel.Eated == 1)
                        Isdanger[x][y].beshoot_after_persuit[d] = 0;
                    else Isdanger[x][y].beshoot_after_persuit[d] = rel.beshoot;
                }
#ifdef DEBUG2
        if (turnID == 1)
        {
            for (x = 0; x < gameField.height; x++)
            {
                for (y = 0; y < gameField.width; y++)
                {
                    for (d = 0; d < 4; d++)
                        cerr << Isdanger[x][y].beshoot_after_persuit[d];
                    cerr << " ";
                }
                cerr << endl;
            }
            cerr << Isdanger[1][3].beshoot_after_persuit[2] << endl;
            cerr << Isdanger[1][3].deadend << endl;
            cerr << deadEnd[1][3] << endl;
        }
#endif // DEBUG2

    }

    void Escape_main(int hunter)
    {
        ;
    }
};
namespace Pacman
{
    FieldProp Eat::make_pos(int r, int c)
    {
        FieldProp fp;
        fp.row = r;
        fp.col = c;
        return fp;
    }

    Eat::Eat(int _my_id)
    {
        my_id = _my_id;
        gf = &gameField;

        // ���ɾ�̬�Ķ����ͱ�
        for (int i = 0; i < gf->height; i++)
            for (int j = 0; j < gf->width; j++)
            {
                my_map[i][j].r = i, my_map[i][j].c = j;
                my_map[i][j].static_type = gf->fieldStatic[i][j];
                my_map[i][j].safe = true;

                for (int d = 0; d < 4; d++)
                {
                    int nxt_x = Nmodulo(i + dir4[d][0]);
                    int nxt_y = Mmodulo(j + dir4[d][1]);
                    if (!(my_map[i][j].static_type & direction2OpposingWall[d]))
                    {
                        // cout << "(" << i << ", " << j << ") -> (" << nxt_x << ", " << nxt_y << ")" << endl;
                        my_map[i][j].nxt.push_back(&my_map[nxt_x][nxt_y]);
                    }
                }
            }

        for (int i = 0; i < gf->height; i++)
            for (int j = 0; j < gf->width; j++)
            {
                // ������̬����
                my_map[i][j].content_type = gf->fieldContent[i][j];

                // ��������
                if (gf->fieldContent[i][j] & smallFruit)
                {
                    small_fruit.push_back(make_pos(i, j));
                    fruit.push_back(make_pos(i, j));
                }
                if (gf->fieldContent[i][j] & largeFruit)
                {
                    large_fruit.push_back(make_pos(i, j));
                    fruit.push_back(make_pos(i, j));
                }
            }

        // ���㼴�������Ķ���
        for (int i = 0; i < gf->generatorCount; i++)
        {
            int x = gf->generators[i].row;
            int y = gf->generators[i].col;
            for (int d = 0; d < 8; d++)
            {
                int r = Nmodulo(x + dir8[d][0]);
                int c = Mmodulo(y + dir8[d][1]);
                if (my_map[r][c].static_type & generator || my_map[r][c].content_type & (smallFruit | largeFruit))
                    continue;
                my_map[r][c].future_type |= smallFruit;
                future_fruit.push_back(make_pos(r, c));
            }
        }

        //calc_danger();
        calc_time();
        calc_density();
        calc_largefruit();
        for (int i = 0; i < gf->height; i++)
            for (int j = 0; j < gf->width; j++)
                my_map[i][j].score.calc(my_id);
    }

    void Eat::calc_largefruit()
    {
        for (int i = 0; i < gf->height; i++)
            for (int j = 0; j < gf->width; j++)
            {
                Node* now = &my_map[i][j];
                if (my_map[i][j].content_type & largeFruit)
                {
                    if (gf->turnID + my_map[i][j].dis_info.dis <= 100)
                    {
                        if (gf->turnID + my_map[i][j].dis_info.dis + gf->LARGE_FRUIT_DURATION > 100)
                            now->score.largefruit_score = 100;
                        else
                            now->score.largefruit_score = (my_id == myID ? 20 : 40);
                    }
                }
            }
    }


    /*Direction Eat::solve(double &recommend) // ����һ�������Լ��Ƽ�ֵ���Ƽ�ֵԽ�ߣ�Խ����ȥ�Զ���
    {
        double best_score = -INF;
        Direction best_direction = stay;

        for (int i = 0; i < gf->height; i++)
            for (int j = 0; j < gf->width; j++)
            {
                Node *now = &my_map[i][j];
                if (now->is_fruit() && (i != gf->players[my_id].row || j != gf->players[my_id].col))
                {
                    now->score.calc();
                    if (now->score.tot_score > best_score)
                    {
                        best_score = now->score.tot_score;
                        best_direction = now->dis_info.first_dir;
                    }
                }
            }

        recommend = best_score;
        return best_direction;
    }*/

    /*void Eat::calc_danger()
    {
        const double eaten_p = 0.2; // �Եĸ��ʳ����������
        const double shoot_p = 0.6; // �����ĸ��ʳ����������

        for (int d = 0; d < 4; d++)
        {
            go[d] = true;

            int now_x = gf->players[my_id].row;
            int now_y = gf->players[my_id].col;
            int x = Nmodulo(now_x + dir4[d][0]);
            int y = Mmodulo(now_y + dir4[d][1]);

            for (int from = 0; from < MAX_PLAYER_COUNT; from++)
                if (from != my_id)
                {
                    int from_x = gf->players[from].row;
                    int from_y = gf->players[from].col;

                    // ����Σ��
                    if ((Isdanger[x][y].eaten >> from) & 1)
                    {
                        for (int _d = 0; _d < 4; _d++)
                        {
                            int from_to_x = Nmodulo(from_x + dir4[_d][0]);
                            int from_to_y = Mmodulo(from_y + dir4[_d][1]);
                            if (from_to_x == x && from_to_y == y)
                            {
                                if (predict[from][_d] > eaten_p)
                                {
                                    go[d] = false;
                                    break;
                                }
                            }
                        }
                    }

                    // ����Σ��
                    if (((Isdanger[x][y].shooted >> from) & 1) && !Isdanger[now_x][now_y].dodged)
                    {
                        int dx = x - from_x;
                        int dy = y - from_y;
                        int _d = 0;
                        if (dx < 0 && dy == 0) _d = 0;
                        else if (dx == 0 && dy > 0) _d = 1;
                        else if (dx > 0 && dy == 0) _d = 2;
                        else if (dx == 0 && dy < 0) _d = 3;

                        if (predict[from][_d + 5] > shoot_p)
                            go[d] = false;
                    }

                    if (!go[d]) break;
                }


            if (!go[d])
                my_map[x][y].safe = false;
        }
    }*/

    void Eat::calc_time()
    {
        for (int i = 0; i < gf->height; i++)
            for (int j = 0; j < gf->width; j++)
                my_map[i][j].dis_info.dis = INF;
        Node* start = &my_map[gf->players[my_id].row][gf->players[my_id].col];
        start->dis_info.dis = 0;
        start->dis_info.first_dir.insert(stay);

        queue<Node*> q;
        map<Node*, int> in_queue;
        q.push(start);
        in_queue[start] = 1;

        while (!q.empty())
        {
            Node* now = q.front();
            q.pop();

            for (vector<Node*>::iterator nxt = now->nxt.begin(); nxt != now->nxt.end(); nxt++)
            {
                int w = 1;
                if ((*nxt)->safe && (*nxt)->dis_info.dis >= now->dis_info.dis + w)
                {
                    // cout << (*nxt)->r << " " << (*nxt)->c << endl;
                    (*nxt)->dis_info.dis = now->dis_info.dis + w;

                    // ��¼Direction
                    if (now == start)
                    {
                        int d = 0;
                        for (; d < 4; d++)
                            if ((*nxt)->r == Nmodulo(now->r + dir4[d][0])
                                && (*nxt)->c == Mmodulo(now->c + dir4[d][1]))
                                break;
                        if (d == 4) d = -1;
                        (*nxt)->dis_info.first_dir.insert((Direction)d);
                    }
                    else
                        for (auto iter : now->dis_info.first_dir)
                            (*nxt)->dis_info.first_dir.insert(iter);

                    if (!in_queue.count(*nxt) || !in_queue[*nxt])
                    {
                        q.push(*nxt);
                        in_queue[*nxt] = 1;
                    }
                }
            }

            in_queue[now] = 0;
        }

        int min_dis = INF;
        Node* min_dis_node = NULL;
        for (int i = 0; i < gf->height; i++)
            for (int j = 0; j < gf->width; j++)
            {
                Node* now = &my_map[i][j];
                if (now->is_future_fruit())
                {
                    now->dis_info.dis = max(now->dis_info.dis, gf->generatorTurnLeft);
                }
                if (now->dis_info.dis < min_dis && now->is_fruit())
                {
                    min_dis = min(min_dis, now->dis_info.dis);
                    min_dis_node = now;
                }
            }

        if (min_dis == INF)
            min_dis = 1;
        double sub_rate = min(10.0, 50.0 / min_dis);

        for (int i = 0; i < gf->height; i++)
            for (int j = 0; j < gf->width; j++)
            {
                Node* now = &my_map[i][j];
                now->score.time_score = 100 - sub_rate * now->dis_info.dis;
            }
    }

    void Eat::calc_density()
    {
        for (int i = 0; i < gf->height; i++)
            for (int j = 0; j < gf->width; j++)
            {
                Node* now = &my_map[i][j];
                if (now->safe && now->is_fruit() && now->dis_info.dis < INF)
                {
                    vis.clear();
                    int dfs_result = dfs(now, gf->generatorTurnLeft - now->dis_info.dis, 0);
                    now->score.density_score = (double)100 / DENSITY_STEP * dfs_result;
                }
            }
    }

    int Eat::dfs(Node* now, int gleft, int step)
    {
        if (step >= DENSITY_STEP)
            return 0;
        bool has_fruit = false;
        if (now->is_now_fruit()) has_fruit = true;
        if (gleft <= 0 && now->is_future_fruit()) has_fruit = true;
        if (vis.count(now) && vis[now] > 0) has_fruit = false;
        if (has_fruit) vis[now] = 1;

        // �ȶ���
        int ans = dfs(now, gleft - 1, step + 1);

        // ��ǰ��
        for (vector<Node*>::iterator nxt = now->nxt.begin(); nxt != now->nxt.end(); nxt++)
            ans = max(ans, dfs(*nxt, gleft - 1, step + 1));

        if (has_fruit)
        {
            ans++;
            vis[now] = 0;
        }

        return ans;
    }

    double Eat::get_recommend(int x, int y)
    {
        return my_map[x][y].score.tot_score;
    }
}
#include <cstring>
#include <algorithm>
#include <iostream>

#define P1 2
#define P2 4
#define P3 7
#define P4 10
#define debug(x) cerr<<#x<<"="<<x<<endl

namespace Pacman
{
    bool CheckIsLine(int px, int py, int d)//�ж�����ͬ�Ƿ���һ��ֱ��
    {
        while (Isdanger[px][py].dodged)
        {
            if ((gameField.fieldStatic[px][py] & direction2OpposingWall[d]))
                break;
            px = Nmodulo(px + dir[d][0]);
            py = Mmodulo(py + dir[d][1]);
        }
        if (Isdanger[px][py].dodged)
            return 1;
        else return 0;
    }

    bool kill_vis[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];
    void countdep_dfs(int x, int y, int& max_dep)
    {
        int d, nx, ny;
        kill_vis[x][y] = 1;
        max_dep = max(max_dep, deadEnd[x][y]);

        for (d = 0; d < 4; d++)
            if ((gameField.fieldStatic[x][y] & direction2OpposingWall[d]) == 0)
            {
                nx = Nmodulo(x + dir[d][0]);
                ny = Mmodulo(y + dir[d][1]);
                if (!kill_vis[nx][ny])
                    countdep_dfs(nx, ny, max_dep);
            }
    }

    int killmain()
    {
        Player Me = gameField.players[myID], Prey;
        Escape_result rel;
        const int val[5] = { 0,0,V2,V3,V4 };//��ʾ׷ɱ���һ���ļ�ֵ���泡��ʣ�������仯
        const int lim_of_pursuit[5] = { 0,P1,P2,P3,P4 };//��ʾ��������ʵ���ӽ�ʱ����Զ׷���غ�,0��ʾ��ǰ�غ�
        const bool can_shoot = (gameField.players[myID].strength > 6);
        //�����Ƿ��ܹ���һ��������һ������������Ҳ���ܶ¶�����ͬ�ˡ������ܴ����һ������ô�ڶ����϶���ȱ����

        int d, hx = Me.row, hy = Me.col, px, py, i, j;
        int best_dir = -1, max_shoot = 0, min_step, step_to_kill;

        for (i = 0; i < MAX_PLAYER_COUNT; i++)
        {
            if (!gameField.players[i].dead)
            {
                Prey = gameField.players[i];
                px = Prey.row;
                py = Prey.col;
                if (distance[hx][hy][px][py] == 1 && Prey.strength < Me.strength)
                {
                    d = get_dir(hx, hy, px, py);
                    int max_strength = 0;//���������ܵ����λ�õ���ǿ�����ж�ǿ
                    for (j = 0; j < MAX_PLAYER_COUNT; j++)
                        if (Isdanger[px][py].eaten & (1 << j))
                            max_strength = max(max_strength, gameField.players[j].strength);
                    if (max_strength > Me.strength)//������ǿ�߽��֣���Ϊ�ϲ�
                        continue;

                    rel = escape_route(myID, i);//����������������űȶԷ�ǿ��������ڴ�����ʧȥǰ�򵽶Է�

                    //������������ͬ��������Ͳ������������Ƚ���
                    if (deadEnd[px][py] == deadEnd[hx][hy] + 1)//���类hunter�µ�����ͬ����(prey������ͬ��ȱ�hunter�Ĵ�1)
                    {
                        memset(kill_vis, 0, sizeof(kill_vis));
                        kill_vis[hx][hy] = 1;
                        countdep_dfs(px, py, step_to_kill);
                        step_to_kill -= deadEnd[px][py];
                        if (step_to_kill >= Me.powerUpLeft && (Me.powerUpLeft > 0 && Me.strength <= Prey.strength + gameField.LARGE_FRUIT_ENHANCEMENT))//ɱ����
                            continue;
                        //�����Ѿ��޷���ܹ��ߣ��ҵ���û�л���֮���ˣ������Ƚ��ţ�������·���ӵĻ��п��ܻ���ط�����
                        if (can_shoot && Isdanger[px][py].dodged && Prey.strength <= gameField.SKILL_COST)
                            return d + 4;
                        //����Ӧ��׷��ȥ�ԣ�����Է���Զ����ܱ��������򣨳�ȥһ�д����������������
                        else
                            return d;
                    }
                    if (!can_shoot) continue;

                    if (Isdanger[px][py].dodged)//�����ܴ�����ڴ�ɣ�Ҳ�Ȳ������������
                        return d + 4;

                    if (!rel.Shooted || (Me.powerUpLeft > 0 && Me.strength <= Prey.strength + 10 && rel.Step_to_be_shoot >= Me.powerUpLeft))
                        continue;//���Ż��ߴ������û�˾��Ȳ�׷�ˡ������������ͦ������
                    else if (rel.Step_to_be_shoot + 1 > val[gameField.aliveCount] && rel.beshoot == 1) //̫Զ���Ҽ�ֵ����
                        continue;
                    else if (Me.strength - Prey.strength <= 4 && rel.Step_to_be_shoot > lim_of_pursuit[Me.strength - Prey.strength])
                        continue;//����ʵ���ӽ���׷̫Զ���ױ���ɱ
                    else
                    {
                        if (max_shoot < rel.beshoot)
                        {
                            max_shoot = rel.beshoot,
                                min_step = rel.Step_to_be_shoot;
                            best_dir = d;
                        }
                        else if (max_shoot == rel.beshoot && rel.Step_to_be_shoot < min_step)
                        {
                            max_shoot = rel.beshoot,
                                min_step = rel.Step_to_be_shoot;
                            best_dir = d;
                        }
                    }
                }
            }
        }
        return best_dir;
    }
};
#include <cstring>
#include <vector>
#include <cmath>
#include <algorithm>

namespace Pacman
{

    double predict[MAX_PLAYER_COUNT][MAX_ACTION_COUNT];
    PredictTypeList pGetout, pShootGetout, pShootBack;
    bool pcanact[MAX_PLAYER_COUNT][MAX_ACTION_COUNT];
    int pescaping[MAX_PLAYER_COUNT];
    pActType pisGetout, pisShootGetout, pisShootBack;

    void buildState()
    {
        for (int i = 0; i < MAX_PLAYER_COUNT; ++i)
        {
            if (gameField.players[i].dead)
                continue;
            int sx = gameField.players[i].row;
            int sy = gameField.players[i].col;
            pescaping[i] = 0;
            for (int k = 0; k < MAX_PLAYER_COUNT; ++k)
                if (!gameField.players[k].dead && gameField.players[k].strength > gameField.players[i].strength)
                {
                    int tx = gameField.players[k].row;
                    int ty = gameField.players[k].col;
                    if (distance[sx][sy][tx][ty] == 1)
                        pescaping[i] |= 1 << k;
                }
        }
    }
    void buildCanact()
    {
        for (int i = 0; i < MAX_PLAYER_COUNT; ++i)
        {
            memset(pcanact[i], 0, sizeof(bool) * MAX_ACTION_COUNT);
            if (gameField.players[i].dead)
                continue;
            int sx = gameField.players[i].row;
            int sy = gameField.players[i].col;
            pcanact[i][0] = true;
            for (int j = 0; j < 4; ++j)
                if ((gameField.fieldStatic[sx][sy] & direction2OpposingWall[j]) == 0)
                {
                    int tx = Nmodulo(sx + dir[j][0]);
                    int ty = Mmodulo(sy + dir[j][1]);
                    bool hvstrong = false;
                    for (int k = 0; k < MAX_PLAYER_COUNT; ++k)
                        if (!gameField.players[k].dead && gameField.players[k].strength > gameField.players[i].strength
                            && gameField.players[k].row == tx && gameField.players[k].col == ty)
                            hvstrong = true;
                    if (!hvstrong)
                        pcanact[i][j + 1] = true;
                    if (gameField.players[i].strength > gameField.SKILL_COST)
                        pcanact[i][j + 5] = true;
                }
            if (pescaping[i])
            {
                pcanact[i][0] = false;
                if (straightDepth[sx][sy] == 0)
                    for (int j = shootUp + 1; j < MAX_ACTION_COUNT; ++j)
                        pcanact[i][j] = false;
            }
        }
        for (int i = 0; i < MAX_PLAYER_COUNT; ++i)
        {
            if (gameField.players[i].dead)
                continue;
            int sx = gameField.players[i].row;
            int sy = gameField.players[i].col;
            for (int j = 0; j < 4; ++j)
                if (pcanact[i][j + 5])
                {
                    bool waste = true;
                    for (int k = 0; k < MAX_PLAYER_COUNT && waste; ++k)
                        if (i != k && !gameField.players[k].dead)
                        {
                            int tx = gameField.players[k].row;
                            int ty = gameField.players[k].col;
                            if (seeLine(sx, sy, j, tx, ty))
                            {
                                waste = false;
                                break;
                            }
                            for (int l = 0; l < 4; ++l)
                                if (pcanact[k][l + 1])
                                {
                                    int tx2 = Nmodulo(tx + dir[l][0]);
                                    int ty2 = Mmodulo(ty + dir[l][1]);
                                    if (seeLine(sx, sy, j, tx2, ty2))
                                    {
                                        waste = false;
                                        break;
                                    }
                                }
                        }
                    if (waste)
                        pcanact[i][j + 5] = false;
                }
        }
        for (int i = 0; i < MAX_PLAYER_COUNT; ++i)
            for (int j = 0; j < MAX_PLAYER_COUNT; ++j)
            {
                if (i == j || gameField.players[i].dead || gameField.players[j].dead)
                    continue;
                for (int d1 = 0; d1 < 4; ++d1)
                    for (int d2 = 0; d2 < 4; ++d2)
                        if (pcanact[i][d1 + 1] && pcanact[j][d2 + 1])
                        {
                            int x1, y1, x2, y2;
                            x1 = Nmodulo(gameField.players[i].row + dir[d1][0]);
                            y1 = Mmodulo(gameField.players[i].col + dir[d1][1]);
                            x2 = Nmodulo(gameField.players[j].row + dir[d2][0]);
                            y2 = Mmodulo(gameField.players[j].col + dir[d2][1]);
                            if (x1 == x2 && y1 == y2)
                            {
                                if (gameField.players[i].strength < gameField.players[j].strength)
                                    pcanact[i][d1 + 1] = false;
                            }
                        }
            }
        for (int i = 0; i < MAX_PLAYER_COUNT; ++i)
            if (!gameField.players[i].dead)
            {
                bool noact = true;
                for (int j = 0; j < MAX_ACTION_COUNT; ++j)
                    if (pcanact[i][j])
                    {
                        noact = false;
                        break;
                    }
                if (noact)
                    pcanact[i][0] = true;
            }
    }
    void buildActType()
    {
        int canShoot[MAX_PLAYER_COUNT][FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH] = {};
        for (int i = 0; i < MAX_PLAYER_COUNT; ++i)
        {
            if (gameField.players[i].dead || gameField.players[i].strength <= gameField.SKILL_COST)
                continue;
            int sx = gameField.players[i].row;
            int sy = gameField.players[i].col;
            for (int j = 0; j < 4; ++j)
                if (pcanact[i][j + 5])
                {
                    int x = sx, y = sy;
                    while ((gameField.fieldStatic[x][y] & direction2OpposingWall[j]) == 0)
                    {
                        x = Nmodulo(x + dir[j][0]);
                        y = Mmodulo(y + dir[j][1]);
                        if (x == sx && y == sy)
                            break;
                        canShoot[i][x][y] |= 1 << j;
                    }
                }
        }

        for (int i = 0; i < MAX_PLAYER_COUNT; ++i)
        {
            if (gameField.players[i].dead)
                continue;
            int sx = gameField.players[i].row;
            int sy = gameField.players[i].col;
            double tmpfruitdis = -1;
            for (int j = 0; j < MAX_ACTION_COUNT; ++j)
                if (pcanact[i][j])
                {
                    int tx, ty;
                    if (j == 0 || j - 1 >= shootUp)
                        tx = sx, ty = sy;
                    else
                    {
                        tx = Nmodulo(sx + dir[j - 1][0]);
                        ty = Mmodulo(sy + dir[j - 1][1]);
                    }
                    bool is = false;
                    for (int k = 0; k < MAX_PLAYER_COUNT; ++k)
                        if (k != i && canShoot[k][tx][ty])
                            is = true;
                    if (!is)
                        tmpfruitdis = max(tmpfruitdis, pfruitdis[i][tx][ty]);
                }
            for (int j = 0; j < MAX_ACTION_COUNT; ++j)
                if (pcanact[i][j])
                {
                    int tx, ty;
                    if (j == 0 || j - 1 >= shootUp)
                        tx = sx, ty = sy;
                    else
                    {
                        tx = Nmodulo(sx + dir[j - 1][0]);
                        ty = Mmodulo(sy + dir[j - 1][1]);
                    }
                    int targetposval = pfruitdis[i][tx][ty];
                    const int maxposval = 130;
                    for (int k = 0; k < MAX_PLAYER_COUNT; ++k)
                    {
                        int px = gameField.players[k].row;
                        int py = gameField.players[k].col;
                        if (i != k && !gameField.players[k].dead)
                            if (gameField.players[i].strength > gameField.players[k].strength)
                                if (distance[sx][sy][px][py] > distance[tx][ty][px][py] && distance[tx][ty][px][py] <= 1)
                                {
                                    targetposval = maxposval;
                                    goto breakposval;
                                }
                    }
                breakposval:
                    if ((targetposval > tmpfruitdis&& tmpfruitdis >= 0) || targetposval == maxposval)
                        for (int k = 0; k < MAX_PLAYER_COUNT; ++k)
                            if (k != i && canShoot[k][tx][ty])
                            {
                                if (j - 1 < shootUp)
                                    pisGetout[i][j] = true;
                                for (int d = 0; d < 4; ++d)
                                    if (canShoot[k][tx][ty] & (1 << d))
                                        pisShootGetout[k][d + 5] = true;
                            }

                    if (j - 1 >= shootUp && Isdanger[sx][sy].dodged)
                        for (int p = 0; p < MAX_PLAYER_COUNT; ++p)
                            if (!gameField.players[p].dead && p != i
                                && seeLine(sx, sy, j - 5, gameField.players[p].row, gameField.players[p].col))
                            {
                                pisShootBack[i][j] = true;
                                break;
                            }
                }
        }
    }

    void calcAverage(int pl, double sm, int* lst, int nlst)
    {
        double sf = 0;
        int sx = gameField.players[pl].row;
        int sy = gameField.players[pl].col;
        const double factor1 = 2, factor2 = 2.3;
        double f[MAX_ACTION_COUNT] = {};
        double mx = 0;
        for (int i = 0; i < nlst; ++i)
        {
            int tx, ty;
            if (lst[i] == 0 || lst[i] - 1 >= shootUp)
                tx = sx, ty = sy;
            else
            {
                tx = Nmodulo(sx + dir[lst[i] - 1][0]);
                ty = Mmodulo(sy + dir[lst[i] - 1][1]);
            }
            f[i] = pfruitdis[pl][tx][ty];
            if (!pisShootGetout[pl][lst[i]] && lst[i] - 1 >= shootUp)
                f[i] /= 2;
            f[i] = max(f[i], 1.0);
            mx = max(f[i], mx);
        }
        for (int i = 0; i < nlst; ++i)
        {
            if (fabs(mx - f[i]) < 0.1)
                f[i] = pow(f[i], factor2);
            else
                f[i] = pow(f[i], factor1);
            sf += f[i];
        }
        for (int i = 0; i < nlst; ++i)
            predict[pl][lst[i]] = sm * f[i] / sf;
    }
    void calcShade(int pl, double sm, int* lst, int nlst)
    {
        int lst0[MAX_ACTION_COUNT], nlst0 = 0;
        int lst1[MAX_ACTION_COUNT], nlst1 = 0;
        int sx = gameField.players[pl].row;
        int sy = gameField.players[pl].col;
        for (int i = 0; i < nlst; ++i)
        {
            int tx, ty;
            if (lst[i] == 0 || lst[i] - 1 >= shootUp)
                tx = sx, ty = sy;
            else
            {
                tx = Nmodulo(sx + dir[lst[i] - 1][0]);
                ty = Mmodulo(sy + dir[lst[i] - 1][1]);
            }
            if (Isdanger[tx][ty].shooted & ~(1 << pl))
                lst0[nlst0++] = lst[i];
            else
                lst1[nlst1++] = lst[i];
        }
        if (nlst0 == 0 || nlst1 == 0)
            calcAverage(pl, sm, lst, nlst);
        else
        {
            double p = 0.75;
            calcAverage(pl, sm * p, lst1, nlst1);
            calcAverage(pl, sm * (1 - p), lst0, nlst0);
        }
    }
    void calcShootGetout(int pl, double sm, int* lst, int nlst)
    {
        int lst0[MAX_ACTION_COUNT], nlst0 = 0;
        int lst1[MAX_ACTION_COUNT], nlst1 = 0;
        for (int i = 0; i < nlst; ++i)
        {
            if (pisShootGetout[pl][lst[i]])
                lst1[nlst1++] = lst[i];
            else
                lst0[nlst0++] = lst[i];
        }
        auto tmp = gameField.players[pl];
        if (nlst0 == 0 || nlst1 == 0)
            calcShade(pl, sm, lst, nlst);
        else
        {
            double p;
            if (pShootGetout[pl].sum != 0)
                p = (double)pShootGetout[pl].hit / pShootGetout[pl].sum;
            else
            {
                if (deadEnd[tmp.row][tmp.col] > 1)
                    p = 0.3;
                else
                    p = 0.2;
            }
            if (tmp.powerUpLeft && tmp.strength - gameField.LARGE_FRUIT_ENHANCEMENT <= gameField.SKILL_COST)
                p /= 2;
            calcShade(pl, sm * p, lst1, nlst1);
            calcShade(pl, sm * (1 - p), lst0, nlst0);
        }
    }
    void calcShootLine(int pl, double sm, int* lst, int nlst)
    {
        int lst0[MAX_ACTION_COUNT], nlst0 = 0;
        int lst1[MAX_ACTION_COUNT], nlst1 = 0;
        int sx = gameField.players[pl].row;
        int sy = gameField.players[pl].col;
        for (int i = 0; i < nlst; ++i)
        {
            if (lst[i] - 1 < shootUp)
                lst0[nlst0++] = lst[i];
            else
            {
                int d = lst[i] - 5;
                bool is = false;
                for (int j = 0; j < MAX_PLAYER_COUNT; ++j)
                {
                    if (gameField.players[j].dead)
                        continue;
                    bool must = true;
                    for (int k = 0; k < MAX_ACTION_COUNT; ++k)
                        if (pcanact[j][k])
                        {
                            int tx = gameField.players[j].row;
                            int ty = gameField.players[j].col;
                            if (k != 0 && k - 1 < shootUp)
                            {
                                tx = Nmodulo(tx + dir[k - 1][0]);
                                ty = Mmodulo(ty + dir[k - 1][1]);
                            }
                            if (!seeLine(sx, sy, d, tx, ty))
                            {
                                must = false;
                                break;
                            }
                        }
                    if (must)
                    {
                        is = true;
                        break;
                    }
                }
                if (is)
                    lst1[nlst1++] = lst[i];
                else
                    lst0[nlst0++] = lst[i];
            }
        }
        if (nlst0 == 0 || nlst1 == 0)
            calcShootGetout(pl, sm, lst, nlst);
        else
        {
            double p = 0.7;
            calcShootGetout(pl, sm * p, lst1, nlst1);
            calcShootGetout(pl, sm * (1 - p), lst0, nlst0);
        }
    }
    void calcShootBack(int pl, double sm, int* lst, int nlst)
    {
        int lst0[MAX_ACTION_COUNT], nlst0 = 0;
        int lst1[MAX_ACTION_COUNT], nlst1 = 0;
        for (int i = 0; i < nlst; ++i)
        {
            if (pisShootBack[pl][lst[i]])
                lst1[nlst1++] = lst[i];
            else
                lst0[nlst0++] = lst[i];
        }
        if (nlst0 == 0 || nlst1 == 0)
            calcShootLine(pl, sm, lst, nlst);
        else
        {
            double p;
            if (pShootBack[pl].sum != 0)
                p = (double)pShootBack[pl].hit / pShootBack[pl].sum;
            else
            {
                auto tmp = gameField.players[pl];
                int strength = tmp.strength;
                if (tmp.powerUpLeft > 0)
                    strength -= gameField.LARGE_FRUIT_ENHANCEMENT;
                bool must = Isdanger[tmp.row][tmp.col].dodged && (Isdanger[tmp.row][tmp.col].shooted & ~(1 << pl));
                if (must && strength - gameField.SKILL_COST * 1.5 <= 0 && strength > gameField.SKILL_COST)
                    p = 1;
                else
                    p = 0.2;
            }
            if (p < 0.9 && pescaping[pl])
                p /= 2;

            calcShootLine(pl, sm * p, lst1, nlst1);
            calcShootLine(pl, sm * (1 - p), lst0, nlst0);
        }
    }
    void calcGetout(int pl, double sm, int* lst, int nlst)
    {
        int lst0[MAX_ACTION_COUNT], nlst0 = 0;
        int lst1[MAX_ACTION_COUNT], nlst1 = 0;
        for (int i = 0; i < nlst; ++i)
        {
            if (pisGetout[pl][lst[i]])
                lst1[nlst1++] = lst[i];
            else
                lst0[nlst0++] = lst[i];
        }
        if (nlst0 == 0 || nlst1 == 0)
            calcShootBack(pl, sm, lst, nlst);
        else
        {
            double p;
            if (pGetout[pl].sum != 0)
                p = (double)pGetout[pl].hit / pGetout[pl].sum;
            else
                p = 0.3;
            calcShootBack(pl, sm * p, lst1, nlst1);
            calcShootBack(pl, sm * (1 - p), lst0, nlst0);
        }
    }
    void calcStraight(int pl, double sm, int* lst, int nlst)
    {
        int lst0[MAX_ACTION_COUNT], nlst0 = 0;
        //int lst1[MAX_ACTION_COUNT], nlst1 = 0;
        int sx = gameField.players[pl].row;
        int sy = gameField.players[pl].col;
        for (int i = 0; i < nlst; ++i)
            if (lst[i] - 1 >= shootUp)
                lst0[nlst0++] = lst[i];
            else
            {
                int tx = Nmodulo(sx + dir[lst[i] - 1][0]);
                int ty = Mmodulo(sy + dir[lst[i] - 1][1]);
                if (straightDepth[tx][ty] && !straightDepth[sx][sy] && (Isdanger[tx][ty].shooted & ~(1 << pl)))
                    ;//lst1[nlst1++] = lst[i];
                else
                    lst0[nlst0++] = lst[i];
            }
        if (nlst0 == 0 /*|| nlst1 == 0*/)
            calcGetout(pl, sm, lst, nlst);
        else
            calcGetout(pl, sm, lst0, nlst0);
    }
    void calcDeadEnd(int pl, double sm, int* lst, int nlst)
    {
        int lst0[MAX_ACTION_COUNT], nlst0 = 0;
        //int lst1[MAX_ACTION_COUNT], nlst1 = 0;
        auto tmp = gameField.players[pl];
        int sx = tmp.row;
        int sy = tmp.col;
        int mxHunter = 0;
        for (int i = 0; i < MAX_PLAYER_COUNT; ++i)
            if (pescaping[pl] & (1 << i))
                mxHunter = max(mxHunter, gameField.players[i].strength);
        for (int i = 0; i < nlst; ++i)
            if (lst[i] - 1 >= shootUp)
                lst0[nlst0++] = lst[i];
            else
            {
                int tx = Nmodulo(sx + dir[lst[i] - 1][0]);
                int ty = Mmodulo(sy + dir[lst[i] - 1][1]);
                if (deadEnd[tx][ty] && mxHunter > tmp.strength + cornerFruitSum(tx, ty, maxDeadDepth[tx][ty], tmp.powerUpLeft))
                    ;//lst1[nlst1++] = lst[i];
                else
                    lst0[nlst0++] = lst[i];
            }
        if (nlst0 == 0 /*|| nlst1 == 0*/)
            calcStraight(pl, sm, lst, nlst);
        else
            calcStraight(pl, sm, lst0, nlst0);
    }
    void calcPredictNormal(int pl) /* getout->shootback->shootline->shootgetout */
    {
        int lst[MAX_ACTION_COUNT], nlst = 0;
        for (int i = 0; i < MAX_ACTION_COUNT; ++i)
            if (pcanact[pl][i])
                lst[nlst++] = i;
        calcGetout(pl, 1, lst, nlst);
    }
    void calcPredictEscape(int pl) /* deadend->straight->getout */
    {
        int lst[MAX_ACTION_COUNT], nlst = 0;
        for (int i = 0; i < MAX_ACTION_COUNT; ++i)
            if (pcanact[pl][i])
                lst[nlst++] = i;
        calcDeadEnd(pl, 1, lst, nlst);
    }

    double pfruitdis[MAX_PLAYER_COUNT][FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];
    void calcpfruitdis(int pl, const set<Point>& ignore)
    {
        for (int r = 0; r < gameField.height; ++r)
            for (int c = 0; c < gameField.width; ++c)
                pfruitdis[pl][r][c] = -1e10;
        Eat peat(pl);
        int sx = gameField.players[pl].row;
        int sy = gameField.players[pl].col;
        for (int r = 0; r < gameField.height; ++r)
            for (int c = 0; c < gameField.width; ++c)
            {
                if (ignore.find(Point(r, c)) != ignore.end())
                    continue;
                Node& now = peat.my_map[r][c];
                if (now.is_fruit())
                    for (auto bestdir : now.dis_info.first_dir)
                    {
                        int tx, ty;
                        if (bestdir == stay || bestdir >= shootUp)
                            tx = sx, ty = sy;
                        else
                        {
                            tx = Nmodulo(sx + dir[bestdir][0]);
                            ty = Mmodulo(sy + dir[bestdir][1]);
                        }
                        pfruitdis[pl][tx][ty] = max(pfruitdis[pl][tx][ty], now.score.tot_score);
                    }
            }
    }
    double posexpect[MAX_PLAYER_COUNT][FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];
    void calcposexpect()
    {
        memset(posexpect, 0, sizeof(posexpect));
        for (int i = 0; i < MAX_PLAYER_COUNT; ++i)
        {
            if (gameField.players[i].dead)
                continue;
            int sx = gameField.players[i].row;
            int sy = gameField.players[i].col;
            for (int j = 0; j < MAX_ACTION_COUNT; ++j)
            {
                int tx, ty;
                if (j == 0 || j - 1 >= shootUp)
                    tx = sx, ty = sy;
                else
                {
                    tx = Nmodulo(sx + dir[j - 1][0]);
                    ty = Mmodulo(sy + dir[j - 1][1]);
                }
                posexpect[i][tx][ty] += predict[i][j];
            }
        }
    }

    void initPredict(string& tauntText)
    {
        for (unsigned i = 0; i < MAX_PLAYER_COUNT; ++i)
            if (turnID == 0)
            {
                pGetout[i].hit = 0, pGetout[i].sum = 0;
                pShootGetout[i].hit = 0, pShootGetout[i].sum = 0;
                pShootBack[i].hit = 0, pShootBack[i].sum = 0;
            }
            else
            {
                if (gameField.players[i].dead)
                    continue;

                Json::Value& pValue = lastValue["predict"];
                pGetout[i].hit = pValue["Getout"][i][0u].asInt(), pGetout[i].sum = pValue["Getout"][i][1].asInt();
                pShootGetout[i].hit = pValue["ShootGetout"][i][0u].asInt(), pShootGetout[i].sum = pValue["ShootGetout"][i][1].asInt();
                pShootBack[i].hit = pValue["ShootBack"][i][0u].asInt(), pShootBack[i].sum = pValue["ShootBack"][i][1].asInt();

                unsigned act = gameField.backtrack[turnID - 1].actions[i] + 1;
                Json::Value& acti = pValue["action"][i][act];
                if (acti["isGetout"].asBool())
                {
                    if (pGetout[i].sum == 0)
                        pGetout[i].hit = 2, pGetout[i].sum = 4;
                    else
                    {
                        pGetout[i].hit++;
                        pGetout[i].sum++;
                    }
                }
                else if (acti["smGetout"].asBool())
                {
                    if (pGetout[i].sum == 0)
                        pGetout[i].hit = 1, pGetout[i].sum = 3;
                    else
                    {
                        pGetout[i].sum++;
                        //if (pGetout[i].hit) /* maybe enermy is clever than before */
                        //pGetout[i].sum++;
                    }
                }

                if (acti["isShootGetout"].asBool())
                {
                    if (pShootGetout[i].sum == 0)
                        pShootGetout[i].hit = 2, pShootGetout[i].sum = 2;
                    else
                    {
                        /* enermy is likely to shoot */
                        pShootGetout[i].hit += 2;
                        pShootGetout[i].sum += 2;
                    }
                }
                else if (acti["smShootGetout"].asBool())
                {
                    if (pShootGetout[i].sum == 0)
                        pShootGetout[i].sum = 2;
                    else
                        pShootGetout[i].sum++;
                }

                if (acti["isShootBack"].asBool())
                {
                    if (pShootBack[i].sum == 0)
                        pShootBack[i].hit = 2, pShootBack[i].sum = 2;
                    else
                    {
                        pShootBack[i].hit++;
                        pShootBack[i].sum++;
                    }
                }
                else if (acti["smShootBack"].asBool())
                {
                    if (pShootBack[i].sum == 0)
                        pShootBack[i].sum = 2;
                    else
                        pShootBack[i].sum++;
                }

                tauntText += to_string(i) + ':' + to_string((int)(acti["predict"].asDouble() * 100)) + ' ';
            }

        for (int i = 0; i < MAX_PLAYER_COUNT; ++i)
            if (!gameField.players[i].dead)
                calcpfruitdis(i, set<Point>());
        buildState();
        buildCanact();
        buildActType();

        bool loop = false;
        for (int i = 0; i < MAX_PLAYER_COUNT; ++i)
            if (!gameField.players[i].dead)
            {
                if (pescaping[i])
                    calcPredictEscape(i);
                else
                    calcPredictNormal(i);

                double p2[MAX_ACTION_COUNT];
                if (matchPhase(&gameField, i, p2))
                {
                    const double p = 0.8;
                    for (int j = 0; j < MAX_ACTION_COUNT; ++j)
                        predict[i][j] = predict[i][j] * (1 - p) + p2[j] * p;
                    loop = true;
                }
            }
        if (loop)
            tauntText += "loop?";

        calcposexpect();

        for (unsigned i = 0; i < MAX_PLAYER_COUNT; ++i)
        {
            if (gameField.players[i].dead)
                continue;

            Json::Value& pValue = nextValue["predict"];
            pValue["Getout"][i][0u] = pGetout[i].hit, pValue["Getout"][i][1] = pGetout[i].sum;
            pValue["ShootGetout"][i][0u] = pShootGetout[i].hit, pValue["ShootGetout"][i][1] = pShootGetout[i].sum;
            pValue["ShootBack"][i][0u] = pShootBack[i].hit, pValue["ShootBack"][i][1] = pShootBack[i].sum;

            bool hvGetout = false, hvShootGetout = false, hvShootBack = false;
            for (unsigned act = 0; act < MAX_ACTION_COUNT; ++act)
            {
                if (pisGetout[i][act])
                    hvGetout = true;
                if (pisShootGetout[i][act])
                    hvShootGetout = true;
                if (pisShootBack[i][act])
                    hvShootBack = true;
            }
            for (unsigned act = 0; act < MAX_ACTION_COUNT; ++act)
            {
                Json::Value& acti = pValue["action"][i][act];
                acti["isGetout"] = pisGetout[i][act];
                acti["smGetout"] = hvGetout;
                acti["isShootGetout"] = pisShootGetout[i][act];
                acti["smShootGetout"] = hvShootGetout;
                acti["isShootBack"] = pisShootBack[i][act];
                acti["smShootBack"] = hvShootBack;
                acti["predict"] = predict[i][act];
            }
        }
    }

}
#include <queue>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <vector>
namespace Pacman
{

    vector<Point> cornerEntrance(int x, int y)
    {
        vector<Point> ans;
        while (deadEnd[x][y] != 0)
            for (int i = 0; i < 4; ++i)
                if ((gameField.fieldStatic[x][y] & direction2OpposingWall[i]) == 0)
                {
                    int tx = Nmodulo(x + dir[i][0]);
                    int ty = Mmodulo(y + dir[i][1]);
                    if (deadEnd[tx][ty] < deadEnd[x][y])
                    {
                        x = tx, y = ty;
                        ans.push_back(Point(x, y));
                        if (tx == gameField.players[myID].row && ty == gameField.players[myID].col)
                            return ans;
                        break;
                    }
                }
        return ans;
    }
    void chaseMain(double pchase[MAX_ACTION_COUNT], string& tauntText)
    {
        memset(pchase, 0, sizeof(double) * MAX_ACTION_COUNT);
        auto me = gameField.players[myID];
        double f1[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH] = {}, f2[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH] = {};
        for (int i = 0; i < MAX_PLAYER_COUNT; ++i)
        {
            if (i == myID || gameField.players[i].dead)
                continue;
            auto target = gameField.players[i];
            auto dan = Isdanger[target.row][target.col];
            if (deadEnd[me.row][me.col] < deadEnd[target.row][target.col]
                && me.strength > target.strength + cornerFruitSum(target.row, target.col, maxDeadDepth[target.row][target.col], target.powerUpLeft)
                && distance[me.row][me.col][target.row][target.col] == 1) // �Ѿ�׷����ͬ������ˣ�������׷
                return;
            if (me.strength > gameField.SKILL_COST&& dan.dodged && (dan.shooted & (1 << myID))) // ֱ���Ѿ����Դ��ˣ�������׷
                return;

            for (int j = 0; j - 1 < shootUp; ++j)
                if (pcanact[i][j])
                {
                    int x, y;
                    if (j == 0 || j - 1 >= shootUp)
                        x = target.row, y = target.col;
                    else
                    {
                        x = Nmodulo(target.row + dir[j - 1][0]);
                        y = Mmodulo(target.col + dir[j - 1][1]);
                    }
                    if (Isdanger[x][y].dodged) // ֱ��
                        for (int d = 0; d < 4; ++d)
                        {
                            int ex = x, ey = y;
                            bool through = false;
                            while ((gameField.fieldStatic[ex][ey] & direction2OpposingWall[d]) == 0)
                            {
                                ex = Nmodulo(ex + dir[d][0]);
                                ey = Mmodulo(ey + dir[d][1]);
                                if (ex == x && ey == y)
                                {
                                    through = true;
                                    break;
                                }
                                int my_strength = me.strength;
                                if (me.powerUpLeft > 0 && me.powerUpLeft <= distance[ex][ey][me.row][me.col])
                                    my_strength -= gameField.LARGE_FRUIT_ENHANCEMENT;
                                int his_strength = target.strength;
                                recalc(his_strength, gameField.fieldContent[x][y], target.powerUpLeft);
                                if (straightDepth[x][y] >= distance[ex][ey][me.row][me.col] && my_strength > gameField.SKILL_COST)
                                    if (my_strength > his_strength || distance[x][y][ex][ey] > 1)
                                        f1[ex][ey] += predict[i][j];
                            }
                            if (through)
                                break;
                        }
                    if (deadEnd[x][y]) // ��ͬ
                    {
                        vector<Point> entrance = cornerEntrance(x, y);
                        for (auto e : entrance)
                        {
                            int round = distance[me.row][me.col][e.x][e.y];
                            if (gameField.turnID + round >= 100)
                                continue;
                            if (me.strength > target.strength + cornerFruitSum(x, y, round + maxDeadDepth[x][y], target.powerUpLeft))
                                if (round <= deadEnd[x][y])
                                {
                                    if (e.x == me.row && e.y == me.col)
                                        f2[e.x][e.y] += predict[i][j] * 0.6;
                                    else
                                        f2[e.x][e.y] += predict[i][j];
                                }
                        }
                    }
                }
        }

        bool taunt = false;
        for (int i = -1; i < 4; ++i)
            if (pcanact[myID][i + 1])
            {
                int tx, ty;
                if (i == -1)
                    tx = me.row, ty = me.col;
                else
                {
                    tx = Nmodulo(me.row + dir[i][0]);
                    ty = Mmodulo(me.col + dir[i][1]);
                }
                for (int x = 0; x < gameField.height; ++x)
                    for (int y = 0; y < gameField.width; ++y)
                        if (distance[me.row][me.col][x][y] > distance[tx][ty][x][y] || (tx == x && ty == y))
                        {
                            if (me.strength > gameField.SKILL_COST &&
                                ((me.strength - (me.powerUpLeft ? gameField.LARGE_FRUIT_ENHANCEMENT : 0) > gameField.SKILL_COST || f1[x][y] >= 0.6)))
                                pchase[i + 1] = max(pchase[i + 1], f1[x][y]);
                            pchase[i + 1] = max(pchase[i + 1], f2[x][y]);
                        }
                if (pchase[i + 1] > 1)
                    pchase[i + 1] = 1;
                if (pchase[i + 1] > 0.7)
                    taunt = true;
            }
        for (int i = 0; i < 4; ++i)
            if (pcanact[myID][i + 5])
                pchase[i + 5] = pchase[0];

        if (taunt)
            tauntText += "I'm looking at you.";
    }
}
#include <queue>
#include <set>
#include <vector>

namespace Pacman
{
    set<Point> avoidMain()
    {
        set<Point> ans;
        auto me = gameField.players[myID];

        //���⿪�ֶ���
        vector<Point> large;
        for (int x = 0; x < gameField.height; ++x)
            for (int y = 0; y < gameField.width; ++y)
                if (gameField.fieldContent[x][y] & largeFruit)
                    large.push_back(Point(x, y));
        for (size_t i = 0; i < large.size(); ++i)
        {
            for (size_t j = 0; j < large.size(); ++j)
                for (int d = 0; d < 4; ++d)
                    if (seeLine(large[j].x, large[j].y, d, large[i].x, large[i].y))
                    {
                        for (int p = 0; p < MAX_PLAYER_COUNT; ++p)
                        {
                            auto tmp = gameField.players[p];
                            if (tmp.dead || p == myID)
                                continue;
                            if (distance[tmp.row][tmp.col][large[j].x][large[j].y]
                                <= distance[me.row][me.col][large[i].x][large[i].y])
                            {
                                ans.insert(large[i]);
                                goto nextloop;
                            }
                        }
                    }
        nextloop:
            ;
        }

        //���ӱ�͵
        for (int x = 0; x < gameField.height; ++x)
            for (int y = 0; y < gameField.width; ++y)
                if (gameField.fieldContent[x][y] & (largeFruit | smallFruit))
                {
                    bool ignore = false;
                    for (int i = 0; i < MAX_PLAYER_COUNT && !ignore; ++i)
                    {
                        auto enermy = gameField.players[i];
                        if (i == myID || enermy.dead)
                            continue;
                        int edis = distance[enermy.row][enermy.col][x][y];
                        int mdis = distance[me.row][me.col][x][y];
                        if (enermy.strength >= me.strength)
                        {
                            if (enermy.row == me.row && enermy.col == me.col) // ����غ�
                                if (x == me.row && y == me.col)
                                    ignore = true;
                            if (edis <= 3 && mdis >= edis + 1)
                                ignore = true;
                            //else if (mdis >= edis * 1.4)
                            //    ignore = true;
                        }
                        else
                        {
                            if (edis <= 2 && mdis >= edis + 1)
                                ignore = true;
                            //else if (edis <= 4 && mdis >= edis + 2)
                            //    ignore = true;
                            //else if (mdis >= edis * 1.5)
                            //    ignore = true;
                        }
                    }
                    if (ignore)
                        ans.insert(Point(x, y));
                }
        return ans;
    }
}
namespace Pacman
{

    struct phaseT
    {
        GridContentType content[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];
        int action;
        struct phasePinfo
        {
            bool self, shoot;
            int strong;
            bool operator!= (const phasePinfo& x)const
            {
                return self != x.self || shoot != x.shoot || strong != x.strong;
            }
        } player_info[MAX_PLAYER_COUNT];
    } _phase[MAX_TURN][MAX_PLAYER_COUNT];

    bool phase_relate_player(GameField* gf, int p1, int p2)
    {
        if (p1 == p2)
            return true;
        int sx1 = gf->players[p1].row;
        int sy1 = gf->players[p1].col;
        int sx2 = gf->players[p2].row;
        int sy2 = gf->players[p2].col;
        for (int i = -1; i < 4; ++i)
        {
            if (i != -1 && (gf->fieldStatic[sx1][sy1] & direction2OpposingWall[i]) != 0)
                continue;
            int tx1, ty1;
            if (i == -1)
                tx1 = sx1, ty1 = sy1;
            else
            {
                tx1 = Nmodulo(sx1 + dir[i][0]);
                ty1 = Mmodulo(sy1 + dir[i][1]);
            }
            for (int j = -1; j < 4; ++j)
            {
                if (j != -1 && (gf->fieldStatic[sx2][sy2] & direction2OpposingWall[j]) != 0)
                    continue;
                int tx2, ty2;
                if (i == -1)
                    tx2 = sx2, ty2 = sy2;
                else
                {
                    tx2 = Nmodulo(sx2 + dir[j][0]);
                    ty2 = Mmodulo(sy2 + dir[j][1]);
                }
                for (int d = 0; d < 4; ++d)
                    if (seeLine(tx1, ty1, d, tx2, ty2))
                        return true;
            }
        }
        return false;
    }
    int extractPhase(GameField* gf, int pl, GridContentType ans[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH], phaseT::phasePinfo player_info[MAX_PLAYER_COUNT])
        // ����ֵ��ʾԭʼ����
    {
        int sx = gf->players[pl].row;
        int sy = gf->players[pl].col;
        int quadrant;
        int q1 = sx < (gf->height + 1) / 2, q2 = sy < (gf->width + 1) / 2;
        if (q1)
            quadrant = (q2 ? 0 : 2);
        else
            quadrant = (q2 ? 1 : 3);

        memset(ans, 0, sizeof(GridContentType) * FIELD_MAX_HEIGHT * FIELD_MAX_WIDTH);
        memset(player_info, 0, sizeof(phaseT::phasePinfo) * MAX_PLAYER_COUNT);
        int minfruitdis = 100;
        for (int i = 0; i < gf->height; ++i)
            for (int j = 0; j < gf->width; ++j)
                if (gf->fieldContent[i][j] & (smallFruit | largeFruit))
                    minfruitdis = min(minfruitdis, distance[i][j][sx][sy]);

        for (int i = 0; i < gf->height; ++i)
            for (int j = 0; j < gf->width; ++j)
            {
                auto tmp = gf->fieldContent[i][j];
                bool restore = false;
                if (distance[i][j][sx][sy] <= 3)
                    restore = true;
                else if ((gf->fieldContent[i][j] & (smallFruit | largeFruit)) && distance[i][j][sx][sy] == minfruitdis)
                    restore = true;
                else if (tmp & playerMask)
                {
                    for (int _ = 0; _ < MAX_PLAYER_COUNT; ++_)
                        if ((tmp & (1 << _)) && phase_relate_player(gf, pl, _))
                        {
                            restore = true;
                            break;
                        }
                }

                int r, c;
                if (quadrant == 0 || quadrant == 1)
                    r = i;
                else
                    r = gf->height - 1 - i;
                if (quadrant == 0 || quadrant == 2)
                    c = j;
                else
                    c = gf->width - 1 - j;

                if (restore)
                {
                    ans[r][c] = tmp;
                    for (int _ = 0; _ < MAX_PLAYER_COUNT; ++_)
                        if (tmp & (1 << _))
                        {
                            if (_ == pl)
                                player_info[_].self = true;
                            if (gf->players[_].strength > gf->SKILL_COST)
                                player_info[_].shoot = true;
                            if (gf->players[_].strength > gf->players[pl].strength)
                                player_info[_].strong = 1;
                            else if (gf->players[_].strength < gf->players[pl].strength)
                                player_info[_].strong = -1;
                            else
                                player_info[_].strong = 0;
                        }
                }
                else
                    ans[r][c] = empty;
            }
        return quadrant;
    }

    void storePhase(GameField* gf, int pl)
    {
        int quadrant = extractPhase(gf, pl, _phase[gf->turnID][pl].content, _phase[gf->turnID][pl].player_info);
        _phase[gf->turnID][pl].action = (Direction)transport_quadrant(quadrant, gf->actions[pl], 0);
    }

    bool samePhase(GridContentType a[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH], GridContentType b[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH]
        , phaseT::phasePinfo ainfo[MAX_PLAYER_COUNT], phaseT::phasePinfo binfo[MAX_PLAYER_COUNT])
    {
        for (int i = 0; i < gameField.height; ++i)
            for (int j = 0; j < gameField.width; ++j)
            {
                auto t1 = a[i][j], t2 = b[i][j];
                int l1[MAX_PLAYER_COUNT], l2[MAX_PLAYER_COUNT], n1 = 0, n2 = 0;
                for (int _ = 0; _ < MAX_PLAYER_COUNT; ++_)
                {
                    if (t1 & (1 << _))
                        l1[n1++] = _;
                    if (t2 & (1 << _))
                        l2[n2++] = _;
                }
                if (n1 != n2)
                    return false;
                if ((t1 & ~playerMask) != (t2 & ~playerMask))
                    return false;
                for (int _ = 0; _ < n1; ++_)
                    if (ainfo[l1[_]] != binfo[l2[_]])
                        return false;
            }
        return true;
    }
    bool matchPhase(GameField* gf, int pl, double ans[MAX_ACTION_COUNT])
    {
        GridContentType ext[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];
        phaseT::phasePinfo player_info[MAX_PLAYER_COUNT];
        int quadrant = extractPhase(gf, pl, ext, player_info);
        memset(ans, 0, sizeof(double) * MAX_ACTION_COUNT);
        int sum = 0;
        for (int i = 0; i < gf->turnID; ++i)
            if (samePhase(_phase[i][pl].content, ext, _phase[i][pl].player_info, player_info))
            {
                ++sum;
                ans[transport_quadrant(0, _phase[i][pl].action, quadrant) + 1] += 1;
            }
        if (sum != 0)
            for (int i = 0; i < MAX_ACTION_COUNT; ++i)
                ans[i] /= sum;
        return sum != 0;
    }

}
#include <algorithm>
#include <iostream>
#include <cmath>
using namespace std;
using namespace Pacman;

const double Fix_num[5] = { 0, 0.4, 0.7, 0.9, 1 }; //���Ҳ���Сʱ��Ҫ����
const double Be_Hit_Val[4] = { 0, 8, 7, 6 };//����ʱ����ʧ
const double Hit_Val[4] = { 0, 4, 3, 2 };//�����ʱ���ջ�

void recalcFruit()
{
    int sx = gameField.players[myID].row;
    int sy = gameField.players[myID].col;
    double mineat = 1e300, maxeat = -1e300;
    for (int i = 0; i < MAX_ACTION_COUNT; ++i)
        if (pcanact[myID][i])
        {
            int tx, ty;
            if (i == 0 || i - 1 >= shootUp)
                tx = sx, ty = sy;
            else
            {
                tx = Nmodulo(sx + dir[i - 1][0]);
                ty = Mmodulo(sy + dir[i - 1][1]);
            }
            mineat = min(mineat, pfruitdis[myID][tx][ty]);
            maxeat = max(maxeat, pfruitdis[myID][tx][ty]);
        }
    if (maxeat > 0)
    {
        for (int i = 0; i < MAX_ACTION_COUNT; ++i)
            if (pcanact[myID][i])
            {
                int tx, ty;
                if (i == 0 || i - 1 >= shootUp)
                    tx = sx, ty = sy;
                else
                {
                    tx = Nmodulo(sx + dir[i - 1][0]);
                    ty = Mmodulo(sy + dir[i - 1][1]);
                }
                pfruitdis[myID][tx][ty] = max(pfruitdis[myID][tx][ty], 0.0);
            }
        /*double mineat2 = 0;
        for (int i = 0; i < 4; ++i)
            if (pcanact[myID][i + 1])
                mineat2 = max(mineat2, pfruitdis[myID][Nmodulo(sx + dir[i][0])][Mmodulo(sy + dir[i][1])]);
        pfruitdis[myID][sx][sy] = max(pfruitdis[myID][sx][sy], mineat2 - 20);*/
        return;
    }

    for (int i = 0; i < MAX_ACTION_COUNT; ++i)
        if (pcanact[myID][i])
        {
            int tx, ty;
            if (i == 0 || i - 1 >= shootUp)
                tx = sx, ty = sy;
            else
            {
                tx = Nmodulo(sx + dir[i - 1][0]);
                ty = Mmodulo(sy + dir[i - 1][1]);
            }
            if (fabs(mineat - maxeat) < 1e-3)
                pfruitdis[myID][tx][ty] = 30;
            else
                pfruitdis[myID][tx][ty] = (pfruitdis[myID][tx][ty] - mineat) / (maxeat - mineat) * 30;
        }
}

void fngMain(Direction& action, string& tauntText, string& nextData)
{
    Init_Isdanger();
    initPredict(tauntText);

    int ans = -2;

    int hunter, kill_dir;
    Escape_result Esc_rel;
    double val[MAX_ACTION_COUNT] = {};
    double pchase[MAX_ACTION_COUNT];
    int sx = gameField.players[myID].row;
    int sy = gameField.players[myID].col;

    set<Point> avoidpoints = avoidMain();
    calcpfruitdis(myID, avoidpoints);

    if ((hunter = Isdanger_now()) != -1)
    {
        Esc_rel = escape_route(hunter, myID);
        //cerr << Esc_rel.Shooted << endl;
        if (Esc_rel.Run_dir == -2)
            tauntText = "Goodbye World!";
        else
            tauntText += "eat" + to_string(Esc_rel.Eated) + " dir " + to_string(Esc_rel.Run_dir) + " shooted " + to_string(Esc_rel.Shooted) + " step " + to_string(Esc_rel.Step_to_be_shoot);
        if (Esc_rel.Run_dir == -1)
            tauntText += "kill me now!";
        kill_dir = -1; // should not go to kill someone now
    }
    else if ((kill_dir = killmain()) != -1)
    {
        if (kill_dir < 4)
            tauntText += "You can not escape!";
        else tauntText += "It's high noon!";
    }

    chaseMain(pchase, tauntText);

    recalcFruit();

    bool fear = false;
    for (int i = 0; i < MAX_ACTION_COUNT; ++i)
        if (pcanact[myID][i])
        {
            int tx, ty;
            if (i == 0 || i - 1 >= shootUp)
                tx = sx, ty = sy;
            else
            {
                tx = Nmodulo(sx + dir[i - 1][0]);
                ty = Mmodulo(sy + dir[i - 1][1]);
            }

            val[i] = pfruitdis[myID][tx][ty] / 65; // this is a good fruit-searching algorithm
            //cerr << pfruitdis[myID][tx][ty] <<endl;

            if (hunter != -1 && Esc_rel.Run_dir != -2)
                if (i - 1 == Esc_rel.Run_dir)
                    val[i] += (100 - turnID + gameField.players[myID].strength) / 2;
            if (kill_dir != -1 && i - 1 == kill_dir)
            {
                int plus = 3;
                if (kill_dir < shootUp)
                    for (int i = 0; i < MAX_PLAYER_COUNT; ++i)
                    {
                        auto target = gameField.players[i];
                        if (i == myID || target.dead)
                            continue;
                        if (predict[i][0] > 0.9)
                            if (tx == target.row && ty == target.col)
                                plus = max(plus, target.strength / 2);
                    }
                val[i] += plus;
            }
            val[i] += gameField.SKILL_COST * 0.5 * pchase[i];

            double pdead = 0, phit = 0;

            if (i - 1 >= shootUp && i - 1 != kill_dir)
            {
                int x = tx, y = ty;
                while ((gameField.fieldStatic[x][y] & direction2OpposingWall[i - 5]) == 0)
                {
                    x = Nmodulo(x + dir[i - 5][0]);
                    y = Mmodulo(y + dir[i - 5][1]);
                    if (x == tx && y == ty)
                        break;
                    for (int j = 0; j < MAX_PLAYER_COUNT; ++j)
                    {
                        if (myID == j || gameField.players[j].dead)
                            continue;
                        phit += posexpect[j][x][y];
                    }
                }
                phit = min(phit, 1.0);
                val[i] -= gameField.SKILL_COST;
                val[i] += gameField.SKILL_COST * 1.5 * phit;
                if (gameField.players[myID].powerUpLeft > 0 && gameField.players[myID].strength - gameField.LARGE_FRUIT_ENHANCEMENT <= gameField.SKILL_COST)
                    pdead += 1 - phit;
            }
            for (int j = 0; j < MAX_PLAYER_COUNT; ++j)
            {
                if (myID == j || gameField.players[j].dead)
                    continue;
                for (int k = 0; k < 4; ++k)
                    if (seeLine(gameField.players[j].row, gameField.players[j].col, k, tx, ty))
                    {
                        val[i] -= gameField.SKILL_COST * 1.5 * predict[j][k + 5];
                        if (gameField.players[myID].strength - (gameField.players[myID].powerUpLeft > 0 ? gameField.LARGE_FRUIT_ENHANCEMENT : 0) + phit * gameField.SKILL_COST * 0.5 <= gameField.SKILL_COST * 1.5)
                            pdead += predict[j][k + 5];
                    }
                if (hunter == -1 && kill_dir == -1)
                    for (int k = -1; k < 4; ++k)//Ԥ�����һ��
                    {
                        int px = gameField.players[j].row, py = gameField.players[j].col;
                        if (!pcanact[j][k + 1])
                            continue;
                        if (k != -1)
                            px = Nmodulo(px + dir[k][0]), py = Mmodulo(py + dir[k][1]);
                        int my_strength = gameField.players[myID].strength, his_strength = gameField.players[j].strength;
                        recalc(my_strength, gameField.fieldContent[tx][ty], gameField.players[myID].powerUpLeft);
                        recalc(his_strength, gameField.fieldContent[px][py], gameField.players[j].powerUpLeft);
                        if (Pacman::distance[tx][ty][px][py] == 1)//����һ��֮����������ż������������ż��������ǿ��ż��
                        {
                            double fix, prob;
                            if (abs(my_strength - his_strength) >= 5)
                                fix = 1.0;
                            else fix = Fix_num[abs(my_strength - his_strength)];
                            if (my_strength > his_strength&& my_strength > gameField.SKILL_COST)//��ʱӦ�ýӽ�
                            {
                                prob = predict[j][k + 1] * Hit_Val[gameField.aliveCount - 1] * Isdanger[px][py].beshoot_after_persuit[get_dir(px, py, tx, ty)];
                                prob *= fix;
                                val[i] += prob;
                            }
                            else if (i - 1 < shootUp && his_strength > my_strength)
                            {
                                prob = predict[j][k + 1] * Be_Hit_Val[gameField.aliveCount - 1] * Isdanger[tx][ty].beshoot_after_persuit[get_dir(tx, ty, px, py)];
                                prob += predict[j][k + 1] * 1.0;//һ�������ţ�����ʧһЩ�����������Լ����
                                prob *= fix * 0.7;
                                val[i] -= prob;
                            }
                        }
                        if (Isdanger[tx][ty].dodged && his_strength > gameField.SKILL_COST) //��ǰһ�������ߵ��޷���ܵ�ֱ�߱���
                            if (his_strength > my_strength || Pacman::distance[px][py][tx][ty] > 1)
                                for (int d = 0; d < 4; d++)
                                    if (seeLine(px, py, d, tx, ty))
                                    {
                                        val[i] -= predict[j][k + 1] * gameField.SKILL_COST;
                                        //cerr << "I'am fear!" << endl;
                                        if (predict[j][k + 1] > 0.5)
                                            fear = true;
                                        break;//�������������ܴ򵽣������ظ�����
                                    }
                    }
            }

            if (pdead > 1)
                pdead = 1;
            if (pdead > 0.1)
                val[i] -= pdead * (100 - turnID) / 3;

            if (ans == -2 || val[i] > val[ans + 1])
                ans = i - 1;
        }

    if (ans == -2)
        action = stay;
    else
        action = (Direction)ans;

    if (fear)
        tauntText += "I'm fear!";

    nextData = Json::FastWriter().write(nextValue);
}

int main()
{
    myID = gameField.ReadInput("input.txt", lastData, globalData); // ���룬������Լ�ID
    srand(seed + myID);
    parseLastData(lastData);

    Direction action;
    string tauntText, nextData;
    fngMain(action, tauntText, nextData); // globalData�����ò���

    gameField.WriteOutput(action, tauntText, nextData, globalData);
    return 0;
}