#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include <Geode/modify/PlayLayer.hpp>
#include <prevter.imageplus/include/api.hpp>
#include <Geode/modify/PauseLayer.hpp>
bool hasDoneThisAttempt = false;
#include <fmod.hpp>
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

			if (!CCScene::get()->getChildByType<PauseLayer>(0)->getChildByID("fiveFiveAnim"_spr)) {
				CCScene::get()->getChildByType<PauseLayer>(0)->addChild(graphic);
				auto winSize = CCDirector::get()->getWinSize();
				graphic->setPosition(winSize / 2);
				graphic->setID("fiveFiveAnim"_spr);
				graphic->play();
				graphic->setScale(2.0f);

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

				if (!CCScene::get()->getChildByType<PauseLayer>(0)->getChildByID("fiveFiveAnim"_spr)) {
					CCScene::get()->getChildByType<PauseLayer>(0)->addChild(graphic);
					auto winSize = CCDirector::get()->getWinSize();
					graphic->setPosition(winSize / 2);
					graphic->setID("fiveFiveAnim"_spr);
					graphic->play();
					graphic->setScale(2.0f);

					

				}
				
				

				

			}
		}
		
		PlayLayer::destroyPlayer(player, cause);
	}

	
};

class $modify(FiveFivePauseLayer, PauseLayer) {
    struct Fields {
        FMOD::Channel* m_soundChannel = nullptr;
		bool canUnpause = true;
		~Fields() {
            if (m_soundChannel) {
                m_soundChannel->stop();
            }
        }
    };
	void customSetup() {
        PauseLayer::customSetup();
		Loader::get()->queueInMainThread([this]() {
			if (auto scene = CCScene::get()) {
				if (auto pauseLayer = scene->getChildByType<PauseLayer>(0)) {
					if (pauseLayer->getChildByID("fiveFiveAnim"_spr)) {
						this->schedule(schedule_selector(FiveFivePauseLayer::checkIfDone));
						m_fields->canUnpause = false;

						auto audioFile = Mod::get()->getResourcesDir() / "fivefive.mp3";
						FMOD::Sound* sound = nullptr;
						FMOD::Channel* channel = nullptr;
						FMOD::System* system = FMODAudioEngine::sharedEngine()->m_system;
										
						system->createSound(geode::utils::string::pathToString(audioFile).c_str(), FMOD_DEFAULT, nullptr, &sound);
						system->playSound(sound, nullptr, false, &m_fields->m_soundChannel);
						m_fields->m_soundChannel->setVolume(1.0f);

						if (m_fields->m_soundChannel) { // just to make sure its not null for whatever reason
							m_fields->m_soundChannel->setVolume(1.0f);
						}
						this->scheduleOnce(schedule_selector(FiveFivePauseLayer::canUnpause), 1.0f);


					
												
					}
				}
			}
		});
    }



	void onAnimFinished(CCNode* sender) {
        sender->removeFromParent();
        this->onResume(nullptr);
    }

	void checkIfDone(float dt) {
		if (!CCScene::get()->getChildByType<PauseLayer>(0)->getChildByID("fiveFiveAnim"_spr)) return;
		auto graphic = static_cast<imgp::AnimatedSprite*>(CCScene::get()->getChildByType<PauseLayer>(0)->getChildByID("fiveFiveAnim"_spr));
		
		if (graphic->getCurrentFrame() >= graphic->getFrameCount() - 1) {
			this->unschedule(schedule_selector(FiveFivePauseLayer::checkIfDone));

			graphic->runAction(CCSequence::create(
				CCFadeOut::create(1.f),
				CCCallFuncN::create(this, callfuncN_selector(FiveFivePauseLayer::onAnimFinished)),
				nullptr
			));


			
		}
	}

	void canUnpause(float dt) {
		m_fields->canUnpause = true;
		
		return;
	}

	void onResume(CCObject* sender) {
		if (!m_fields->canUnpause) return;
		PauseLayer::onResume(sender);
	}

};