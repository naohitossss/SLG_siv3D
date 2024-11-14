#include <Siv3D.hpp> // Siv3Dのヘッダーファイル
#include <vector>

using namespace std;

enum class Owner { Player, Enemy, Neutral };

struct ScoreEffect : IEffect
{
	Vec2 m_start;

	int32 m_score;

	Font m_font;

	ScoreEffect(const Vec2& start, int32 score, const Font& font)
		: m_start{ start }
		, m_score{ score }
		, m_font{ font } {}

	bool update(double t) override
	{
		const HSV color{ (180 - m_score * 1.8), 1.0 - (t * 2.0) };

		m_font(m_score).drawAt(m_start.movedBy(0, t * -120), color);

		return (t < 0.5);
	}
};

// 領地を表す構造体
struct Territory {
	Vec2 position;
	int soldiers;
	int growthRate;
	Color color;
	Owner owner;
	vector<reference_wrapper<Territory>> connections;

	void update() {
		addSoldiers();
	}

	void addConnection(Territory& target) {
		connections.push_back(target);
	}

	void draw() const {
		Circle(position, 40).draw(color);
		FontAsset(U"Default")(U"{:02}"_fmt(soldiers)).drawAt(position, Palette::White);
	}

	void addSoldiers() {
		if (owner == Owner::Player || owner == Owner::Enemy) {
			soldiers += growthRate;
		}
	}
};


class Stage {
public:
	int attackSoldier;
	// 攻撃処理
	void attack(Territory& thisTerritory, Territory& target) {
		thisTerritory.soldiers -= attackSoldier;
		if (thisTerritory.color != target.color) {
			if (target.soldiers > 0) {
				target.soldiers -= attackSoldier;

				// 敵の兵士数が0未満なら領地を占領する
				if (target.soldiers <= 0) {
					target.color = thisTerritory.color;
					target.owner = thisTerritory.owner;
					target.soldiers = -target.soldiers;
				}
			}
		}
		else {
			target.soldiers += attackSoldier;
		}
	}
	// 矢印がクリックされたときの攻撃ボタン処理
	void attackButton(Territory& thisTerritory, Territory& target, const Line& arrowLine) {
		if (thisTerritory.owner == Owner::Player && arrowLine.intersects(Cursor::PosF())) {
			Cursor::RequestStyle(CursorStyle::Hand); // カーソルを「手」の形に変更
			if (MouseL.down() && thisTerritory.soldiers > attackSoldier) {
				attack(thisTerritory, target); // クリック時に攻撃を実行
			}
		}
	}

	void drawArrow(Territory& thisTerritory) {
		for (auto& target : thisTerritory.connections) {
			Vec2 direction = (target.get().position - thisTerritory.position).normalized();
			double length = thisTerritory.position.distanceFrom(target.get().position) - 40; // 領地のサイズを考慮
			Line arrowLine{ thisTerritory.position + (direction * (length / 2 + 25)), thisTerritory.position + (direction * (length - 5)) };

			// 矢印の色設定と描画
			if (thisTerritory.owner == Owner::Player) {
				arrowLine.drawArrow(30, SizeF{ 20, 40 }, Palette::Green);
				attackButton(thisTerritory, target.get(), arrowLine);
			}
			else {
				arrowLine.drawArrow(30, SizeF{ 20, 40 }, Palette::White);
			}
		}
	}
	void checkWin(const vector<reference_wrapper<Territory>>& territories, bool& isWin, bool& isLose) {
		bool playerExists = false;
		bool enemyExists = false;

		for (const auto& territory : territories) {
			if (territory.get().owner == Owner::Player) {
				playerExists = true;
			}
			if (territory.get().owner == Owner::Enemy) {
				enemyExists = true;
			}
		}

		isWin = !enemyExists;  // 敵の領地が全て無くなったら勝利
		isLose = !playerExists;  // プレイヤーの領地が全て無くなったら敗北
	}

	void Setattacksoldier(int soldiernum) {
		attackSoldier = soldiernum;
	}

};

class Game {
public:
	vector<reference_wrapper<Territory>> territories = { playerTerritory, neutralTerritory, enemyTerritory };

	// 領地の接続設定
	void setConnection() {
		playerTerritory.addConnection(neutralTerritory);
		neutralTerritory.addConnection(playerTerritory);
		enemyTerritory.addConnection(neutralTerritory);
		neutralTerritory.addConnection(enemyTerritory);
	}

	// 領地をリストにまとめる


	vector<reference_wrapper<Territory>> getTerritories() {
		return territories;

	}

	void gameReset(vector<reference_wrapper<Territory>>& resetTerritories) {

		territories = { playerTerritory, neutralTerritory, enemyTerritory };
	}

private:
	Territory playerTerritory{ Vec2(100, 300), 10, 3, Palette::Blue,Owner::Player };
	Territory enemyTerritory{ Vec2(700, 300), 10, 3, Palette::Red, Owner::Enemy };
	Territory neutralTerritory{ Vec2(300, 200), 5, 3, Palette::Gray, Owner::Neutral };

};

void Main()
{
	Effect effect;
	Stage stage;
	Game game;
	const Font font{ FontMethod::MSDF, 48, Typeface::Heavy };

	FontAsset::Register(U"Default", 20);

	const int growthTime = 3;
	Stopwatch stopwatch;
	stopwatch.start();
	bool ifWin, ifLose = false;


	game.setConnection();
	stage.Setattacksoldier(1);
	// 領地をリストにまとめる
	vector<reference_wrapper<Territory>> territories = game.getTerritories();

	// ゲームループ
	while (System::Update())
	{
		if (SimpleGUI::Button(U"Rest", Vec2{ 20, 20 })) {
			territories = game.getTerritories();
			//FontAsset(U"Default")(U"Rest").drawAt(Scene::Center(), Palette::Yellow);
		}
		if (SimpleGUI::Button(U"SetSolider to 5", Vec2{ 20, 500 })) {
			stage.Setattacksoldier(5);
		}
		if (SimpleGUI::Button(U"SetSolider to 10", Vec2{ 20, 540 })) {
			stage.Setattacksoldier(10);
		}
		// 勝敗判定
		stage.checkWin(territories, ifWin, ifLose);

		if (ifWin) {
			FontAsset(U"Default")(U"YOU WIN").drawAt(Scene::Center(), Palette::Yellow);
		}
		else if (ifLose) {
			FontAsset(U"Default")(U"YOU LOSE").drawAt(Scene::Center(), Palette::Red);
		}
		else {

			for (auto& territory : territories) {
				territory.get().draw();
				stage.drawArrow(territory.get());
			}

			// 一定時間ごとに兵士を増加
			if (stopwatch.s() >= growthTime) {
				for (auto territory : territories) {
					territory.get().update();
					if (territory.get().owner != Owner::Neutral)
						effect.add<ScoreEffect>(territory.get().position, territory.get().growthRate, font);
				}
				stopwatch.restart();
			}
			effect.update();
		}
	}
}
