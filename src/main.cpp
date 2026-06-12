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


		if (GJBaseGameLayer::get()->m_isPlatformer) return true;
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
		bool onlyOnDeath = Mod::get()->getSettingValue<bool>("onlyondeath");

		if (GJBaseGameLayer::get()->m_isPlatformer) return;
		if (GJBaseGameLayer::get()->m_isTestMode && !onlyOnDeath) return;
		if (this->m_isPracticeMode && !onlyOnDeath) return;
		
		if (percent >= number && !hasDoneThisAttempt) {
			log::info("onlyondeath: {}", onlyOnDeath);

			if (onlyOnDeath) return;
			pauseGame(false);
			
			auto pauseLayer = CCScene::get()->getChildByType<PauseLayer>(0);
			if (!pauseLayer) return;
			if (onlyOnDeath) return;

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
				addResumeButton();
				auto winSize = CCDirector::get()->getWinSize();
				graphic->setPosition(winSize / 2);
				graphic->setID("fiveFiveAnim"_spr);
				graphic->play();
				if (Mod::get()->getSettingValue<bool>("fullscreen")) {
					auto contentSize = graphic->getContentSize();

					graphic->setScaleX(winSize.width / contentSize.width);
					graphic->setScaleY(winSize.height / contentSize.height);
				} else {
					graphic->setScale(2.0f);
				}

				
				

			}
			
	

			hasDoneThisAttempt = true;
		}
	}

	void destroyPlayer(PlayerObject* player, GameObject* cause) {
		if (Mod::get()->getSettingValue<bool>("onlyondeath")) {
			if (cause == m_anticheatSpike) {
				PlayLayer::destroyPlayer(player, cause);
				return;
			}
			log::info("runningfromdestroyplayer?");
			if (GJBaseGameLayer::get()->m_isPlatformer) {
				PlayLayer::destroyPlayer(player, cause);
				return;
			}

			if (GJBaseGameLayer::get()->m_isTestMode) {
				PlayLayer::destroyPlayer(player, cause);
				return;
			}
			if (this->m_isPracticeMode) {
				PlayLayer::destroyPlayer(player, cause);
				return;
			}
			if (player != m_player1 && player != m_player2) {
				PlayLayer::destroyPlayer(player, cause);
				return;
			}

			float percent = getCurrentPercent();
			if (percent >= number && percent < number + 1) {
				this->scheduleOnce(schedule_selector(FiveFivePlayLayer::setUpAndAddGraphic), 0.3f);
			}
		}
		PlayLayer::destroyPlayer(player, cause);
		
	}

	void setUpAndAddGraphic(float dt) {
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
			addResumeButton();
			auto winSize = CCDirector::get()->getWinSize();
			graphic->setPosition(winSize / 2);
			graphic->setID("fiveFiveAnim"_spr);
			graphic->play();
			if (Mod::get()->getSettingValue<bool>("fullscreen")) {
				auto contentSize = graphic->getContentSize();

				graphic->setScaleX(winSize.width / contentSize.width);
				graphic->setScaleY(winSize.height / contentSize.height);
			} else {
				graphic->setScale(2.0f);
			}
			

			

		}
	}

	void addResumeButton() {
		auto winSize = CCDirector::get()->getWinSize();
		auto buttonMenu = CCMenu::create();

		auto buttonSprite = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
		buttonSprite->setScale(0.5);
		auto button = CCMenuItemSpriteExtra::create(
			buttonSprite,
			this,
			menu_selector(FiveFivePlayLayer::continueGame)
		);

		buttonMenu->setID("five-five-resume-menu"_spr);
		buttonMenu->setPosition({ winSize.width / 2, 45.f });

		buttonMenu->addChild(button);
		OverlayManager::get()->addChild(buttonMenu);
	}

	void continueGame(CCObject* sender) {
		auto scene = CCScene::get();
		auto pauseLayer = scene->getChildByType<PauseLayer>(0);
		Loader::get()->queueInMainThread([]() {
			if (auto pauseLayer = CCScene::get()->getChildByType<PauseLayer>(0)) {
				pauseLayer->onResume(nullptr);
			}
   		});
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
			if (auto fiveFive = OverlayManager::get()->getChildByID("fiveFiveAnim"_spr)) {
				fiveFive->removeFromParent();
			}
			if (auto resumeButton = OverlayManager::get()->getChildByID("five-five-resume-menu"_spr)) {
				resumeButton->removeFromParent();
			}
        }
    };
	void customSetup() {
        PauseLayer::customSetup();
		Loader::get()->queueInMainThread([this]() {
			if (auto scene = CCScene::get()) {
				if (auto pauseLayer = scene->getChildByType<PauseLayer>(0)) {
					if (OverlayManager::get()->getChildByID("fiveFiveAnim"_spr)) {
						this->schedule(schedule_selector(FiveFivePauseLayer::checkIfDone));
						m_fields->canUnpause = false;

						auto audioFile = Mod::get()->getResourcesDir() / "fivefive.mp3";
						FMOD::Sound* sound = nullptr;
						FMOD::Channel* channel = nullptr;
						FMOD::System* system = FMODAudioEngine::sharedEngine()->m_system;
										
						system->createSound(geode::utils::string::pathToString(audioFile).c_str(), FMOD_DEFAULT, nullptr, &sound);
						system->playSound(sound, nullptr, false, &m_fields->m_soundChannel);

						if (m_fields->m_soundChannel) { // just to make sure its not null for whatever reason
							m_fields->m_soundChannel->setVolume(Mod::get()->getSettingValue<float>("volume"));
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
		if (!OverlayManager::get()->getChildByID("fiveFiveAnim"_spr)) return;
		auto graphic = static_cast<imgp::AnimatedSprite*>(OverlayManager::get()->getChildByID("fiveFiveAnim"_spr));

		#ifdef GEODE_IS_WINDOWS
		if (GetActiveWindow() == WindowFromDC(wglGetCurrentDC())) {
			FMOD::Channel* channel = nullptr;
			m_fields->m_soundChannel->setPaused(false);
			graphic->play();
		} else {
			m_fields->m_soundChannel->setPaused(true);
			graphic->pause();

		}
		#endif
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
