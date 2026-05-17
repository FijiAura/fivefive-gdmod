#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include <Geode/modify/PlayLayer.hpp>
#include <prevter.imageplus/include/api.hpp>
#include <Geode/modify/PauseLayer.hpp>
bool hasDoneThisAttempt = false;

class $modify(FiveFivePlayLayer, PlayLayer) {
	bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
		if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
			return false;
		}

		hasDoneThisAttempt = false;
		if (!Mod::get()->getSettingValue<bool>("onlyondeath")) {
			this->schedule(schedule_selector(FiveFivePlayLayer::updateLogic));
		}
		
		
		return true;
	}

	void resetLevel() {
		hasDoneThisAttempt = false;
		PlayLayer::resetLevel();
	}


	void updateLogic(float dt) {
		float percent = getCurrentPercent();
		if (percent >= 55 && !hasDoneThisAttempt) {
			pauseGame(false);
			
			auto pauseLayer = CCScene::get()->getChildByType<PauseLayer>(0);
			if (!pauseLayer) return;


			auto plChildren = pauseLayer->getChildren();
			for (auto child : CCArrayExt<CCNode*>(plChildren)) {
				if (child) {
					child->setVisible(false);
				}
			}
			
			
			auto graphicSetup = CCSprite::create("fivefive.webp"_spr);
			auto graphic = imgp::AnimatedSprite::from(graphicSetup);
			graphic->setForceLoop(false);
			graphic->setCurrentFrame(0);

			if (!OverlayManager::get()->getChildByID("fiveFiveAnim"_spr)) {
				OverlayManager::get()->addChild(graphic);
				auto winSize = CCDirector::get()->getWinSize();
				graphic->setPosition(winSize / 2);
				graphic->setID("fiveFiveAnim"_spr);
				graphic->play();
				graphic->setScale(2.0f);
				FMODAudioEngine::get()->playEffect("fivefive.mp3"_spr);

			}
			
			

			hasDoneThisAttempt = true;
		}
	}

	void destroyPlayer(PlayerObject* player, GameObject* cause) {
		if (Mod::get()->getSettingValue<bool>("onlyondeath")) {
			float percent = getCurrentPercent();
		
			if (percent >= 55 && percent < 56) {
				pauseGame(false);
				
				auto pauseLayer = CCScene::get()->getChildByType<PauseLayer>(0);
				if (!pauseLayer) return;


				auto plChildren = pauseLayer->getChildren();
				for (auto child : CCArrayExt<CCNode*>(plChildren)) {
					if (child) {
						child->setVisible(false);
					}
				}
				
				
				auto graphicSetup = CCSprite::create("fivefive.webp"_spr);
				auto graphic = imgp::AnimatedSprite::from(graphicSetup);
				graphic->setForceLoop(false);
				graphic->setCurrentFrame(0);

				if (!OverlayManager::get()->getChildByID("fiveFiveAnim"_spr)) {
					OverlayManager::get()->addChild(graphic);
					auto winSize = CCDirector::get()->getWinSize();
					graphic->setPosition(winSize / 2);
					graphic->setID("fiveFiveAnim"_spr);
					graphic->play();
					graphic->setScale(2.0f);
					FMODAudioEngine::get()->playEffect("fivefive.mp3"_spr);

				}
				
				

				

			}
		}
		
		PlayLayer::destroyPlayer(player, cause);
	}

	
};

class $modify(FiveFivePauseLayer, PauseLayer) {

	void customSetup() {
        PauseLayer::customSetup();

        this->schedule(schedule_selector(FiveFivePauseLayer::checkIfDone));
    }


	void onAnimFinished(CCNode* sender) {
        sender->removeFromParent();
        this->onResume(nullptr);
    }

	void checkIfDone(float dt) {
		if (!OverlayManager::get()->getChildByID("fiveFiveAnim"_spr)) return;
		auto graphic = static_cast<imgp::AnimatedSprite*>(OverlayManager::get()->getChildByID("fiveFiveAnim"_spr));
		
		if (graphic->getCurrentFrame() >= graphic->getFrameCount() - 1) {
			this->unschedule(schedule_selector(FiveFivePauseLayer::checkIfDone));

			graphic->runAction(CCSequence::create(
				CCFadeOut::create(1.f),
				CCCallFuncN::create(this, callfuncN_selector(FiveFivePauseLayer::onAnimFinished)),
				nullptr
			));


			
		}
	}

	void onResume(CCObject* sender) {
		if (OverlayManager::get()->getChildByID("fiveFiveAnim"_spr)) return; // makes 100% sure you cant step the game forward a frame by clicking space >:3
		PauseLayer::onResume(sender);
	}

	void tryQuit(CCObject* sender) {
		if (OverlayManager::get()->getChildByID("fiveFiveAnim"_spr)) return;
		PauseLayer::tryQuit(sender);
		hasDoneThisAttempt = false;

	}
};