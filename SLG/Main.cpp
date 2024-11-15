#include <Siv3D.hpp>
#include <vector>
#include <random>

using namespace std;
random_device rd;
mt19937 gen(rd());

enum class Owner { Player, Enemy, Neutral };

struct ScoreEffect : IEffect {
	Vec2 m_start;
	int32 m_score;
	Font m_font;

	ScoreEffect(const Vec2& start, int32 score, const Font& font)
		: m_start{ start }
		, m_score{ score }
		, m_font{ font } {}

	bool update(double t) override {
		const HSV color{ (180 - m_score * 1.8), 1.0 - (t * 2.0) };
		m_font(m_score).drawAt(m_start.movedBy(0, t * -120), color);
		return (t < 0.5);
	}
};

class Territory {
private:
	Vec2 m_position;
	int m_soldiers;
	int m_growthRate;
	Color m_color;
	Owner m_owner;
	vector<reference_wrapper<Territory>> m_connections;

public:
	Territory(Vec2 pos, int soldiers, int growth, Color color, Owner owner)
		: m_position(pos)
		, m_soldiers(soldiers)
		, m_growthRate(growth)
		, m_color(color)
		, m_owner(owner) {}

	void update() {
		if (m_owner != Owner::Neutral) {
			m_soldiers += m_growthRate;
		}
	}

	void addConnection(Territory& target) {
		m_connections.push_back(target);
	}

	void draw() const {
		Circle(m_position, 40).draw(m_color);
		FontAsset(U"Default")(U"{:02}"_fmt(m_soldiers)).drawAt(m_position, Palette::White);
	}

	// ゲッター・セッター
	const Vec2& getPosition() const { return m_position; }
	int getSoldiers() const { return m_soldiers; }
	void setSoldiers(int value) { m_soldiers = value; }
	const Color& getColor() const { return m_color; }
	void setColor(const Color& color) { m_color = color; }
	Owner getOwner() const { return m_owner; }
	void setOwner(Owner owner) { m_owner = owner; }
	const vector<reference_wrapper<Territory>>& getConnections() const { return m_connections; }
};

class RandomEnemyAI {
private:
	int random(int low, int high)
	{
		uniform_int_distribution<> dist(low, high);
		return dist(gen);
	}


public:
	int ramdomSoldiernum(int maxSolider) {
		return Random(0, maxSolider);
	}

	Territory &ramdomAttack(Territory &territory) {
		return territory.getConnections()[Random(territory.getConnections().size() - 1)].get();
	}



};

class Stage {
private:
	int m_attackSoldiers;

public:
	Stage(int initialAttackSoldiers = 1) : m_attackSoldiers(initialAttackSoldiers) {}

	void attack(Territory& source, Territory& target) {
		if (source.getSoldiers() < m_attackSoldiers) {
			return;
		}

		source.setSoldiers(source.getSoldiers() - m_attackSoldiers);

		if (source.getColor() != target.getColor()) {
			if (target.getSoldiers() > 0) {
				target.setSoldiers(target.getSoldiers() - m_attackSoldiers);

				if (target.getSoldiers() <= 0) {
					target.setColor(source.getColor());
					target.setOwner(source.getOwner());
					target.setSoldiers(-target.getSoldiers());
				}
			}
		}
		else {
			target.setSoldiers(target.getSoldiers() + m_attackSoldiers);
		}
	}

	void enemyAttack(Territory& source, Territory& target,int attackSoilder) {
		if (source.getSoldiers() < attackSoilder) {
			return;
		}

		source.setSoldiers(source.getSoldiers() - attackSoilder);

		if (source.getColor() != target.getColor()) {
			if (target.getSoldiers() > 0) {
				target.setSoldiers(target.getSoldiers() - attackSoilder);

				if (target.getSoldiers() <= 0) {
					target.setColor(source.getColor());
					target.setOwner(source.getOwner());
					target.setSoldiers(-target.getSoldiers());
				}
			}
		}
		else {
			target.setSoldiers(target.getSoldiers() + attackSoilder);
		}
	}

	void drawArrowsAndHandleClicks(Territory& territory) {
		for (auto& targetRef : territory.getConnections()) {
			Territory& target = targetRef.get();
			Vec2 direction = (target.getPosition() - territory.getPosition()).normalized();
			double length = territory.getPosition().distanceFrom(target.getPosition()) - 40;

			Line arrowLine{
				territory.getPosition() + (direction * (length / 2 + 25)),
				territory.getPosition() + (direction * (length - 5))
			};

			if (territory.getOwner() == Owner::Player) {
				arrowLine.drawArrow(30, SizeF{ 20, 40 }, Palette::Green);

				if (arrowLine.intersects(Cursor::PosF())) {
					Cursor::RequestStyle(CursorStyle::Hand);
					if (MouseL.down() && territory.getSoldiers() >= m_attackSoldiers) {
						attack(territory, target);
					}
				}
			}
			else {
				arrowLine.drawArrow(30, SizeF{ 20, 40 }, Palette::White);
			}
		}
	}

	void checkGameOver(const vector<reference_wrapper<Territory>>& territories, bool& isWin, bool& isLose) {
		bool playerExists = false;
		bool enemyExists = false;

		for (const auto& territory : territories) {
			if (territory.get().getOwner() == Owner::Player) playerExists = true;
			if (territory.get().getOwner() == Owner::Enemy) enemyExists = true;
		}

		isWin = !enemyExists;
		isLose = !playerExists;
	}

	void setAttackSoldiers(int value) { m_attackSoldiers = value; }
	int getAttackSoldiers() const { return m_attackSoldiers; }
};

class Game {
private:
	Territory m_playerTerritory;
	Territory m_neutralTerritory;
	Territory m_enemyTerritory;
	vector<reference_wrapper<Territory>> m_territories;
	vector<reference_wrapper<Territory>> m_enemyTerritories;
	Stage m_stage;
	RandomEnemyAI m_randomAI;
	Effect m_effect;
	Font m_font;
	Stopwatch m_growthTimer;
	Stopwatch m_AITimer;
	const int GROWTH_TIME = 3;
	const int AI_TIME = 2;
	bool m_isWin = false;
	bool m_isLose = false;

public:
	Game()
		: m_playerTerritory(Vec2(100, 300), 10, 3, Palette::Blue, Owner::Player)
		, m_neutralTerritory(Vec2(300, 200), 5, 3, Palette::Gray, Owner::Neutral)
		, m_enemyTerritory(Vec2(700, 300), 10, 3, Palette::Red, Owner::Enemy)
		, m_font(FontMethod::MSDF, 48, Typeface::Heavy)
	{
		FontAsset::Register(U"Default", 20);
		setupConnections();
		resetGame();
		m_growthTimer.start();
		m_AITimer.start();
	}

	void setupConnections() {
		m_playerTerritory.addConnection(m_neutralTerritory);
		m_neutralTerritory.addConnection(m_playerTerritory);
		m_neutralTerritory.addConnection(m_enemyTerritory);
		m_enemyTerritory.addConnection(m_neutralTerritory);

		m_territories = {
			m_playerTerritory,
			m_neutralTerritory,
			m_enemyTerritory
		};
	}

	void resetGame() {
		// 領地を初期状態に戻す
		m_playerTerritory = Territory(Vec2(100, 300), 10, 3, Palette::Blue, Owner::Player);
		m_neutralTerritory = Territory(Vec2(300, 200), 5, 3, Palette::Gray, Owner::Neutral);
		m_enemyTerritory = Territory(Vec2(700, 300), 10, 3, Palette::Red, Owner::Enemy);

		setupConnections();
		m_isWin = m_isLose = false;
		m_growthTimer.restart();
		m_AITimer.restart();
	}

	void update() {
		drawUI();

		m_stage.checkGameOver(m_territories, m_isWin, m_isLose);

		if (m_isWin || m_isLose) {
			drawGameOverScreen();
			return;
		}

		for (auto& territory : m_territories) {
			territory.get().draw();
			m_stage.drawArrowsAndHandleClicks(territory);
		}

		if (m_growthTimer.s() >= GROWTH_TIME) {
			updateGrowth();
			m_growthTimer.restart();
		}

		if (m_AITimer.s() >= AI_TIME) {
			enemyAttack();
			m_AITimer.restart();
		}

		m_effect.update();
	}

private:

	void getEnemyTerritory() {
		for (auto& territory : m_territories) {
			if (territory.get().getOwner() == Owner::Enemy) m_enemyTerritories.push_back(territory);
		}
		}
	void enemyAttack() {
		m_enemyTerritories.clear();
		getEnemyTerritory();
		for (auto& territory : m_enemyTerritories) {
			Territory& source = territory.get();
			Territory& target = m_randomAI.ramdomAttack(source);
			int attackSoldier = m_randomAI.ramdomSoldiernum(source.getSoldiers());
			m_stage.enemyAttack(source, target, attackSoldier);
		}


	}
	void drawUI() {
		if (SimpleGUI::Button(U"リセット", Vec2{ 20, 20 })) {
			resetGame();
		}

		const Array<std::pair<String, int>> soldierButtons = {
			{U"兵士数を1に設定", 1},
			{U"兵士数を5に設定", 5},
			{U"兵士数を10に設定", 10}
		};

		for (int i = 0; i < soldierButtons.size(); ++i) {
			if (SimpleGUI::Button(soldierButtons[i].first, Vec2{ 20, 460 + i * 40 })) {
				m_stage.setAttackSoldiers(soldierButtons[i].second);
			}
		}
	}

	void drawGameOverScreen() {
		if (m_isWin) {
			FontAsset(U"Default")(U"勝利！").drawAt(Scene::Center(), Palette::Yellow);
		}
		else if (m_isLose) {
			FontAsset(U"Default")(U"敗北...").drawAt(Scene::Center(), Palette::Red);
		}
	}

	void updateGrowth() {
		for (auto& territory : m_territories) {
			territory.get().update();
			if (territory.get().getOwner() != Owner::Neutral) {
				m_effect.add<ScoreEffect>(territory.get().getPosition(), 3, m_font);
			}
		}
	}
};

void Main() {
	Game game;
	while (System::Update()) {
		game.update();
	}
}
