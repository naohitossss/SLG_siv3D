#include <Siv3D.hpp> // Siv3Dのヘッダーファイル
#include <vector>

struct Territory {
	Vec2 position;
	int soldiers;
	int growthRate;
	Color color;
	bool isPlayer, isNeutral, isEnemy;
	std::vector<Territory*> connections;
	int attackSoldier = 1;

	// 毎秒兵士を増やす
	void update() {
		addSoldiers();
	}

	void addConnection(Territory* target) {
		connections.push_back(target);
	}

	void attack(Territory* target) {
		// 敵の領地に攻撃する場合
		if (color != target->color) {
			soldiers -= attackSoldier;

			// 攻撃目標の兵士数がまだ残っている場合
			if (target->soldiers > 0) {
				target->soldiers -= attackSoldier;

				// 目標の兵士数が0未満になった場合は領地を占領する
				if (target->soldiers < 0) {
					target->color = color;     // 領地の色を自国の色に変更
					target->isPlayer = true;
					target->isNeutral = false;
					target->isEnemy = !isPlayer;

					// 負の兵士数を自国に変換
					target->soldiers = -target->soldiers;
				}
			}
		}
	}

	// 領地を描画
	void draw() {
		Circle(position, 40).draw(color);
		FontAsset(U"Default")(U"{:02}"_fmt(soldiers)).drawAt(position, Palette::White);
		for (auto* target : connections) {
			Vec2 direction = (target->position - position).normalized();
			double length = position.distanceFrom(target->position) - 40; // 領地のサイズを考慮
			Line arrowLine{ position + (direction * (length / 2 + 25)), position + (direction * (length - 5)) };

			// 矢印の描画（プレイヤーなら緑、それ以外は白）
			if (isPlayer) {
				arrowLine.drawArrow(20, SizeF{ 20, 40 }, Palette::Green);
			}
			else {
				arrowLine.drawArrow(20, SizeF{ 20, 40 }, Palette::White);
			}

			// プレイヤーの領地かつカーソルが矢印上にある場合にクリック処理を許可
			if (isPlayer && arrowLine.intersects(Cursor::PosF())) {
				Cursor::RequestStyle(CursorStyle::Hand); // カーソルを「手」の形に

				if (MouseL.down()) {
					attack(target); // targetに攻撃
				}
			}
		}
	}

	// 毎秒兵士を増やす関数
	void addSoldiers() {
		if (isPlayer || isEnemy) soldiers += growthRate;
	}
};



void Main()
{
	FontAsset::Register(U"Default", 20);

	const int growthTime = 3;
	Stopwatch stopwatch; // ストップウォッチを宣言
	stopwatch.start();

	// 領地の設定
	Territory playerTerritory{ Vec2(100, 300), 10, 5, Palette::Blue, true, false, false };
	Territory enemyTerritory{ Vec2(700, 300), 10, 5, Palette::Red, false, false, true };
	Territory neutralTerritory{ Vec2(300, 200), 5, 0, Palette::Gray, false, true, false };

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
		// 領地を描画
		for (auto* territory : territories) {
			territory->draw();
		}

		// growthTime経過ごとに兵士を増加
		if (stopwatch.s() >= growthTime) {
			for (auto* territory : territories) {
				territory->addSoldiers();
			}
			stopwatch.restart(); // タイマーをリセット（ループ外に移動）
		}
	}
}

