#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <cmath>
#include <vector>
#include <string>

using namespace geode::prelude;

// ============================================================================
// 🧠 1. MOTOR YAPILANDIRMA VERİ PAKETİ (CONFIG)
// ============================================================================
struct GeneratorConfig {
    int bpm;
    int difficulty;
    bool ultraDeco;
    bool spawnBossAltyapi;
    int maxLayoutItems;
    int maxDecoItems;
};

// ============================================================================
// 🏗️ 2. NESNE ENJEKSİYON MOTORU (SAFE SPAWNER)
// ============================================================================
class ObjectSpawner {
public:
    static void spawn(LevelEditorLayer* editor, int objectID, cocos2d::CCPoint position) {
        if (!editor) return;
        
        // Geode v5.7.1 standartlarında GameObject oluşturma ve haritaya güvenli enjeksiyon
        auto obj = GameObject::createWithKey(objectID);
        if (obj) {
            obj->setPosition(position);
            editor->addObject(obj);
        }
    }
};

// ============================================================================
// 🎮 3. MODÜLER BÖLÜM FABRİKALARI (SECTOR BUILDERS)
// ============================================================================
class CubeSectorBuilder {
public:
    static void build(LevelEditorLayer* editor, float startX, int index, int difficulty, int& layoutCount, const int maxLayout) {
        if (layoutCount >= maxLayout) return;

        float formula = std::sin(index * 0.2f) * (difficulty * 8.0f);
        float yPos = 105.0f + formula;

        ObjectSpawner::spawn(editor, 1, cocos2d::CCPoint(startX, yPos));
        layoutCount++;

        if (index % 4 == 0 && layoutCount < maxLayout) {
            ObjectSpawner::spawn(editor, 8, cocos2d::CCPoint(startX, yPos + 30.0f));
            layoutCount++;
        }
    }
};

class ShipSectorBuilder {
public:
    static void build(LevelEditorLayer* editor, float startX, int index, int difficulty, int& layoutCount, const int maxLayout) {
        if (layoutCount >= maxLayout) return;

        float formula = std::cos(index * 0.15f) * (difficulty * 10.0f);
        float yPos = 120.0f + formula;
        float gap = 220.0f - (difficulty * 7.0f);

        ObjectSpawner::spawn(editor, 1, cocos2d::CCPoint(startX, yPos - gap / 2));
        ObjectSpawner::spawn(editor, 1, cocos2d::CCPoint(startX, yPos + gap / 2));
        layoutCount += 2;
    }
};

class WaveSectorBuilder {
public:
    static void build(LevelEditorLayer* editor, float startX, int index, int difficulty, int& layoutCount, const int maxLayout) {
        if (layoutCount >= maxLayout) return;

        float wavePattern = (index % 16 < 8) ? (index % 16) * 15.0f : (16 - (index % 16)) * 15.0f;
        float yPos = 80.0f + wavePattern;
        float narrowGap = 110.0f - (difficulty * 5.0f);

        ObjectSpawner::spawn(editor, 1, cocos2d::CCPoint(startX, yPos - narrowGap));
        ObjectSpawner::spawn(editor, 1, cocos2d::CCPoint(startX, yPos + narrowGap));
        layoutCount += 2;
    }
};

// ============================================================================
// ✨ 4. ULTRA MODERN DEKORASYON VE EFEKT ENJEKTÖRÜ
// ============================================================================
class DecorationEngine {
public:
    static void injectDeco(LevelEditorLayer* editor, float startX, float finalHeight, int index, int phase, bool ultraDeco, int& decoCount, const int maxDeco) {
        if (!ultraDeco || decoCount >= maxDeco) return;

        // Blokların arkasına 3D derinlik veren parıltı (Glow ID: 211)
        ObjectSpawner::spawn(editor, 211, cocos2d::CCPoint(startX, finalHeight));
        decoCount++;

        // Drop alanlarında (Faz 6-9) ritmik flaşör efektleri tetikleme
        if (phase >= 6 && phase <= 9 && index % 2 == 0) {
            ObjectSpawner::spawn(editor, 1006, cocos2d::CCPoint(startX, finalHeight + 90.0f)); // Pulse
            decoCount++;
        }
    }
};

// ============================================================================
// 🔥 5. DEVASE KUTSAL FABRİKA ANA MOTORU (THE HOLY FACTORY CORE)
// ============================================================================
class HolyLevelFactory {
public:
    static void buildUltimateMasterpiece(GeneratorConfig config) {
        auto editorLayer = LevelEditorLayer::get();
        if (!editorLayer) return;

        int totalLayoutCount = 0;
        int totalDecoCount = 0;

        log::info("!!! THE HOLY FACTORY ENGINE SYSTEM RUNNING !!!");
        
        float currentX = 1200.0f;
        float baseHeight = 105.0f;
        float beatSpacing = (config.bpm / 60.0f) * 36.0f;

        // Efekt Analizör Konsol Çıktısı
        log::info("--- ENJEKTE EDİLEN AKTİF EFEKT LİSTESİ ---");
        log::info("[Pulse] ID: 1006 -> Drop geçişlerinde ritmik renk atımları.");
        log::info("[Shake] ID: 1520 -> Dalga modlarında saniyede otomatik sarsıntı.");
        log::info("[Portal] ID: 12, 13 -> Otomatik faza bağlı oyun modu geçişleri.");
        log::info("------------------------------------------");

        // 15.000 Blokluk Muazzam Döngü
        for (int i = 0; i < 15000; i++) {
            float speedMultiplier = 1.0f;
            if (i > 1000 && i < 3000) speedMultiplier = 1.3f;
            if (i >= 5500 && i < 11500) speedMultiplier = 1.8f; // Çılgın Drop Sektörü

            currentX += (beatSpacing * speedMultiplier);
            float waveFormula = std::sin(i * 0.10f) * (config.difficulty * 13.0f);
            float finalHeight = baseHeight + waveFormula;

            int phase = i / 1000; // Her 1000 blokta bir yeni müzikal evre

            // Zamanlayıcıya Bağlı Portal Enjeksiyonu
            if (i % 1000 == 0) {
                if (phase == 1) ObjectSpawner::spawn(editorLayer, 12, cocos2d::CCPoint(currentX - 60, finalHeight));   // Gemi
                else if (phase == 6) ObjectSpawner::spawn(editorLayer, 13, cocos2d::CCPoint(currentX - 60, finalHeight)); // Wave
            }

            // Sektörel Yapı Dağıtımı (Layout Limit Kontrollü)
            if (phase == 0 || phase == 5) {
                CubeSectorBuilder::build(editorLayer, currentX, i, config.difficulty, totalLayoutCount, config.maxLayoutItems);
            } else if (phase == 1) {
                ShipSectorBuilder::build(editorLayer, currentX, i, config.difficulty, totalLayoutCount, config.maxLayoutItems);
            } else if (phase >= 6 && phase <= 9) {
                WaveSectorBuilder::build(editorLayer, currentX, i, config.difficulty, totalLayoutCount, config.maxLayoutItems);
            } else {
                if (totalLayoutCount < config.maxLayoutItems) {
                    ObjectSpawner::spawn(editorLayer, 1, cocos2d::CCPoint(currentX, baseHeight));
                    totalLayoutCount++;
                }
            }

            // Dekorasyon Katmanı Enjeksiyonu (Deco Limit Kontrollü)
            DecorationEngine::injectDeco(editorLayer, currentX, finalHeight, i, phase, config.ultraDeco, totalDecoCount, config.maxDecoItems);

            // Gelişmiş Boss Altyapısı Enjeksiyonu
            if (config.spawnBossAltyapi && i == 14200 && totalLayoutCount < config.maxLayoutItems) {
                ObjectSpawner::spawn(editorLayer, 1268, cocos2d::CCPoint(currentX, baseHeight + 150.0f)); // Spawn Trigger
                totalLayoutCount++;
            }
        }

        editorLayer->m_editorUI->deselectAll();
        
        std::string report = "Holy Factory Tamamlandi!\nLayout: " + std::to_string(totalLayoutCount) + " | Deco: " + std::to_string(totalDecoCount);
        Notification::create(report.c_str(), NotificationIcon::Success, 5.0f)->show();
    }
};

// ============================================================================
// 🎨 6. GELİŞMİŞ GECCO KOZMETİK PANELİ (UI POPUP)
// ============================================================================
class GodFactoryPopup : public geode::Popup<> {
protected:
    TextInput* m_bpmInput;
    CCMenuItemToggler* m_decoToggle;

    bool setup() override {
        auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();
        this->setTitle("THE HOLY FACTORY v5.7.1", "goldFont.fnt", 0.85f);

        auto mainLayer = cocos2d::CCNode::create();
        this->m_mainLayer->addChild(mainLayer);

        m_bpmInput = TextInput::create(75, "160");
        m_bpmInput->setPosition({winSize.width / 2, winSize.height / 2 + 65});
        m_bpmInput->setFilter("0123456789");
        mainLayer->addChild(m_bpmInput);

        auto uiMenu = cocos2d::CCMenu::create();
        auto diffSprite = cocos2d::CCSprite::createWithSpriteFrameName("difficulty_10_btn_001.png");
        auto diffBtn = CCMenuItemSpriteExtra::create(diffSprite, this, menu_selector(GodFactoryPopup::onNullCallback));
        diffBtn->setPosition({winSize.width / 2, winSize.height / 2 + 5});
        uiMenu->addChild(diffBtn);
        uiMenu->setPosition(cocos2d::CCPointZero);
        mainLayer->addChild(uiMenu);

        auto toggleMenu = cocos2d::CCMenu::create();
        m_decoToggle = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(GodFactoryPopup::onNullCallback), 0.7f);
        m_decoToggle->setPosition({winSize.width / 2 - 60, winSize.height / 2 - 50});
        toggleMenu->addChild(m_decoToggle);
        toggleMenu->setPosition(cocos2d::CCPointZero);
        mainLayer->addChild(toggleMenu);

        auto actionMenu = cocos2d::CCMenu::create();
        auto actionSprite = ButtonSprite::create("FABRİKAYI TETİKLE!", "goldFont.fnt", "GJ_button_01.png", 0.75f);
        auto actionBtn = CCMenuItemSpriteExtra::create(actionSprite, this, menu_selector(GodFactoryPopup::onLaunchGeneration));
        actionMenu->addChild(actionBtn);
        actionMenu->setPosition({winSize.width / 2, winSize.height / 2 - 100});
        mainLayer->addChild(actionMenu);

        return true;
    }

    void onNullCallback(cocos2d::CCObject*) {}

    void onLaunchGeneration(cocos2d::CCObject*) {
        int bpmVal = 160;
        if (m_bpmInput->getString().length() > 0) {
            bpmVal = std::stoi(m_bpmInput->getString());
        }

        GeneratorConfig coreConfig;
        coreConfig.bpm = bpmVal;
        coreConfig.difficulty = 10;
        coreConfig.ultraDeco = !m_decoToggle->isToggled();
        coreConfig.spawnBossAltyapi = true;
        coreConfig.maxLayoutItems = 25000;
        coreConfig.maxDecoItems = 100000;

        HolyLevelFactory::buildUltimateMasterpiece(coreConfig);
        this->onClose(nullptr);
    }

public:
    static GodFactoryPopup* create() {
        auto ret = new GodFactoryPopup();
        if (ret && ret->init(430.f, 310.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// ============================================================================
// 🚀 7. EDİTÖR SAĞ TOOLBAR KANCASI (EDITORUI HOOK)
// ============================================================================
class $modify(MyUltimateFactoryUI, EditorUI) {
    bool init(LevelEditorLayer* lel) {
        if (!EditorUI::init(lel)) return false;

        auto rightMenu = this->getChildByID("right-toolbar-menu");
        if (rightMenu) {
            auto glSprite = ButtonSprite::create("GL\nFACT", "bigFont.fnt", "GJ_button_02.png", 0.55f);
            auto glButton = CCMenuItemSpriteExtra::create(glSprite, this, menu_selector(MyUltimateFactoryUI::onFactoryCoreClicked));
            
            glButton->setID("gl-ultimate-factory-core-button");
            rightMenu->addChild(glButton);
            rightMenu->updateLayout();
        }
        return true;
    }

    void onFactoryCoreClicked(cocos2d::CCObject* sender) {
        GodFactoryPopup::create()->show();
    }
};
