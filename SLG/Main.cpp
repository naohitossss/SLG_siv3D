#include <Siv3D.hpp> // Siv3Dのヘッダーファイル
#include <vector>

struct Territory {
	Vec2 position;
	int soldiers;
	int growthRate;
	Color color;
	bool isPlayer, isNeutral,isEnemy;
	std::vector<Territory*> connections;
	const timer;

	// 毎秒兵士を増やす
	void update() {
		if(timer == 60){
		if (!isNeutral) soldiers += growthRate;
		}
	}
	void addConnection(Territory* target) {
		connections.push_back(target);
	}
	// 領地を描画
	void draw() const {
		Circle(position, 40).draw(color);
		FontAsset(U"Default")(U"{:02}"_fmt(soldiers)).drawAt(position, Palette::White);
	}
	void AttactButton(const Rect& rect,bool ifPlayer)
	{
		rect.draw(ColorF{ 0.3, 0.7, 1.0 });
	}
};



void Main()
{

	FontAsset::Register(U"Default", 20);

	// 領地の設定
	Territory playerTerritory{ Vec2(100, 300), 10, 1, Palette::Blue, true };
	Territory enemyTerritory{ Vec2(700, 300), 10, 1, Palette::Red, false };
	Territory neutralTerritory{ Vec2(300, 200), 5, 0, Palette::Gray, false };

	// 領地の接続設定
	playerTerritory.addConnection(&neutralTerritory);
	neutralTerritory.addConnection(&playerTerritory);
	enemyTerritory.addConnection(&neutralTerritory);
	neutralTerritory.addConnection(&enemyTerritory);

	// 領地をリストにまとめる
	std::vector<Territory*> territories = { &playerTerritory, &neutralTerritory, &enemyTerritory };

	// ゲームループ
	while (System::Update())
	{
		for (auto *territory : territories) {
			territory->update();
			territory->draw();
		}
		for (auto* territory : territories) {
			for (auto* target : territory->connections) {
				Vec2 direction = (target->position - territory->position).normalized();
				double length = territory->position.distanceFrom(target->position) - 40; // 領地のサイズを考慮
				Shape2D::Arrow(territory->position+ (direction * (length - 5)), territory->position+ (direction * (length/2 + 25)), 10, Vec2(20, 20)).draw(Palette::White);
			}
		}

	}
}
