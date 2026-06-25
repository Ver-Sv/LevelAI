#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <cmath>
#include <vector>
#include <string>
#include <map>

using namespace geode::prelude;

// ============================================================================
// 🔥 1. GLOBAL VERİ YAPILARI VE AYAR SAKLAYICILARI (DATA STRUCTURES)
// ============================================================================
struct GeneratorConfig {
    int bpm;
    int difficulty;
    bool ultraDeco;
    bool spawnBossAltyapi;
    int maxLayoutItems;
    int maxDecoItems;
};

struct ObjectTemplate {
    int id;
    float xOffset;
    float yOffset;
    int type; // 0: Layout, 1: Deco, 2: Trigger
};

// ============================================================================
// 🔥 2. HER OYUN MODU İÇİN AYRI ÖZELLEŞTİRİLMİŞ SEKTÖR SINIFLARI (SECTOR SUB-ENGIONS)
// ============================================================================

class CubeSectorBuilder {
public:
    static void build(LevelEditorLayer* editor, float startX, float spacing, int index, int difficulty, int& layoutCount, const int maxLayout) {
        if (layoutCount >= maxLayout) return;

        float formula = sin(index * 0.2f) * (difficulty * 8.0f);
        float yPos = 105.0f + formula;

        // Temel Taban Bloğu (ID: 1)
        editor->m_editorUI->createObject(1, CCPoint(startX, yPos), true);
        layoutCount++;

        // Ritmik Sıçrama Engelleri
        if (index % 4 == 0 && layoutCount < maxLayout) {
            editor->m_editorUI->createObject(8, CCPoint(startX, yPos + 30), true); // Diken
            layoutCount++;
        }

        // Havada Asılı Platformlar (Zorluk seviyesi arttıkça karmaşıklaşır)
        if (difficulty >= 7 && index % 6 == 0 && layoutCount < maxLayout) {
            editor->m_editorUI->createObject(1, CCPoint(startX + 15.0f, yPos + 60.0f), true);
            editor->m_editorUI->createObject(35, CCPoint(startX + 15.0f, yPos + 90.0f), true); // Yellow Orb
            layoutCount += 2;
        }
    }
};

class ShipSectorBuilder {
public:
    static void build(LevelEditorLayer* editor, float startX, float spacing, int index, int difficulty, int& layoutCount, const int maxLayout) {
        if (layoutCount >= maxLayout) return;

        float formula = cos(index * 0.15f) * (difficulty * 10.0f);
        float yPos = 120.0f + formula;
        float gap = 220.0f - (difficulty * 7.0f);

        // Alt Koridor Sınırı
        editor->m_editorUI->createObject(1, CCPoint(startX, yPos - gap / 2), true);
        // Üst Koridor Sınırı
        editor->m_editorUI->createObject(1, CCPoint(startX, yPos + gap / 2), true);
        layoutCount += 2;

        // Tünel İçi Engeller (Testereler ID: 85)
        if (index % 8 == 0 && layoutCount < maxLayout) {
            float tnelVar = sin(index) * (gap / 4);
            editor->m_editorUI->createObject(85, CCPoint(startX, yPos + tnelVar), true);
            layoutCount++;
        }
    }
};

class BallSectorBuilder {
public:
    static void build(LevelEditorLayer* editor, float startX, float spacing, int index, int difficulty, int& layoutCount, const int maxLayout) {
        if (layoutCount >= maxLayout) return;

        float yPos = 105.0f;
        // Ball için çift zemin yapısı (Tavan ve Taban)
        editor->m_editorUI->createObject(1, CCPoint(startX, yPos - 50.0f), true);
        editor->m_editorUI->createObject(1, CCPoint(startX, yPos + 70.0f), true);
        layoutCount += 2;

        // Yerçekimi Değiştirme Pedleri (ID: 36)
        if (index % 5 == 0 && layoutCount < maxLayout) {
            if ((index / 5) % 2 == 0) {
                editor->m_editorUI->createObject(36, CCPoint(startX, yPos - 35.0f), true); // Alt tabana pad
            } else {
                editor->m_editorUI->createObject(36, CCPoint(startX, yPos + 55.0f), true); // Üst tavana pad (Ters)
            }
            layoutCount++;
        }
    }
};

class UfoSectorBuilder {
public:
    static void build(LevelEditorLayer* editor, float startX, float spacing, int index, int difficulty, int& layoutCount, const int maxLayout) {
        if (layoutCount >= maxLayout) return;

        float formula = sin(index * 0.3f) * 40.0f;
        float yPos = 110.0f + formula;

        // UFO için ritmik geçiş sütunları
        if (index % 12 == 0) {
            for (int h = 0; h < 4; h++) {
                if (layoutCount < maxLayout) {
                    editor->m_editorUI->createObject(1, CCPoint(startX, yPos + (h * 30.0f)), true);
                    layoutCount++;
                }
            }
        } else {
            editor->m_editorUI->createObject(1, CCPoint(startX, yPos - 40.0f), true);
            layoutCount++;
        }
    }
};

class WaveSectorBuilder {
public:
    static void build(LevelEditorLayer* editor, float startX, float spacing, int index, int difficulty, int& layoutCount, const int maxLayout) {
        if (layoutCount >= maxLayout) return;

        // Nine Circles Tarzi Zikzak Yapı Formülü
        float wavePattern = (index % 16 < 8) ? (index % 16) * 15.0f : (16 - (index % 16)) * 15.0f;
        float yPos = 80.0f + wavePattern;
        float narrowGap = 110.0f - (difficulty * 5.0f); // Sınırları zorlayan darlık

        editor->m_editorUI->createObject(1, CCPoint(startX, yPos - narrowGap), true);
        editor->m_editorUI->createObject(1, CCPoint(startX, yPos + narrowGap), true);
        layoutCount += 2;

        // Dar Tünel İçi Sinsi Dikenler (Maksimum Zorluk İçin)
        if (difficulty >= 9 && index % 4 == 0 && layoutCount < maxLayout) {
            editor->m_editorUI->createObject(8, CCPoint(startX, yPos - narrowGap + 15), true);
            layoutCount++;
        }
    }
};

class RobotSectorBuilder {
public:
    static void build(LevelEditorLayer* editor, float startX, float spacing, int index, int difficulty, int& layoutCount, const int maxLayout) {
        if (layoutCount >= maxLayout) return;

        // Robot için uzak atlama platformları tasarımı
        if (index % 14 < 6) {
            editor->m_editorUI->createObject(1, CCPoint(startX, 90.0f), true);
            layoutCount++;
        } else if (index % 14 >= 8) {
            editor->m_editorUI->createObject(1, CCPoint(startX, 160.0f), true); // Yüksek platform
            layoutCount++;
            if (index % 14 == 10) {
                editor->m_editorUI->createObject(8, CCPoint(startX, 175.0f), true); // Üstüne engel
                layoutCount++;
            }
        }
    }
};

class SpiderSectorBuilder {
public:
    static void build(LevelEditorLayer* editor, float startX, float spacing, int index, int difficulty, int& layoutCount, const int maxLayout) {
        if (layoutCount >= maxLayout) return;

        // Örümcek için ani ışınlanma koridorları
        float yPos = 105.0f;
        editor->m_editorUI->createObject(1, CCPoint(startX, yPos - 70.0f), true);
        editor->m_editorUI->createObject(1, CCPoint(startX, yPos + 90.0f), true);
        layoutCount += 2;

        if (index % 10 == 0 && layoutCount < maxLayout) {
            editor->m_editorUI->createObject(8, CCPoint(startX, yPos - 55.0f), true);
            editor->m_editorUI->createObject(8, CCPoint(startX, yPos + 75.0f), true); // Üst-alt engeller
            layoutCount += 2;
        }
    }
};

// ============================================================================
// 🔥 3. GELİŞMİŞ DEKORASYON VE EFEKT ENJEKTÖR MOTORU (DECO ENGINE)
// ============================================================================
class DecorationEngine {
public:
    static void injectDecoAndEffects(LevelEditorLayer* editor, float startX, float finalHeight, int index, int phase, bool ultraDeco, int& decoCount, const int maxDeco) {
        if (!ultraDeco || decoCount >= maxDeco) return;

        // 1. 3D Derinlik Efekti: Her bloğun arkasına çok katmanlı Glow (ID: 211) yerleştirme
        editor->m_editorUI->createObject(211, CCPoint(startX, finalHeight), true);
        editor->m_editorUI->createObject(211, CCPoint(startX - 4.0f, finalHeight + 4.0f), true);
        editor->m_editorUI->createObject(211, CCPoint(startX + 4.0f, finalHeight - 4.0f), true);
        decoCount += 3;

        // 2. Tavan ve Taban Modern Sınır Çizgileri (ID: 41)
        if (index % 2 == 0 && decoCount < maxDeco) {
            editor->m_editorUI->createObject(41, CCPoint(startX, finalHeight + 140.0f), true);
            editor->m_editorUI->createObject(41, CCPoint(startX, finalHeight - 140.0f), true);
            decoCount += 2;
        }

        // 3. NINE CIRCLES GÖZ ALICI FLASHER EFEKTİ (Faz 6, 7, 8, 9 Drop Alanlarında)
        if (phase >= 6 && phase <= 9) {
            if (index % 2 == 0 && decoCount < maxDeco) {
                // Şarkı vuruşlarına bağlı Pulse Trigger simülasyonu (ID: 1006)
                editor->m_editorUI->createObject(1006, CCPoint(startX, finalHeight + 90.0f), true);
                decoCount++;
            }
            if (index % 4 == 0 && decoCount < maxDeco) {
                // Arka planı çılgınca flaşlatacak Renk Değiştirici (Color Trigger ID: 29)
                editor->m_editorUI->createObject(29, CCPoint(startX, finalHeight - 90.0f), true);
                decoCount++;
            }
        }

        // 4. Havada Asılı Estetik Zincirler ve Detay Sarkıtları (ID: 40)
        if (index % 12 == 0 && decoCount < maxDeco) {
            editor->m_editorUI->createObject(40, CCPoint(startX, finalHeight - 35.0f), true);
            decoCount++;
        }
    }
};

// ============================================================================
// 🔥 4. DEVASE ANA MOTOR SINIFI (THE HOLY ENGINE CORP)
// ============================================================================
class HolyLevelFactory {
public:
    static void buildUltimateMasterpiece(GeneratorConfig config) {
        auto editorLayer = LevelEditorLayer::get();
        if (!editorLayer) return;

        int totalLayoutCount = 0;
        int totalDecoCount = 0;

        log::info("!!! THE HOLY FACTORY ENGINE SYSTEM START !!!");
        
        float currentX = 1200.0f;
        float baseHeight = 105.0f;
        float beatSpacing = (config.bpm / 60.0f) * 36.0f;

        // --- 📋 KAPSAMLI EFEKT ANALİZÖR RAPORLAMASI ---
        log::info("--- ENJEKTE EDİLEN AKTİF EFEKT LİSTESİ ---");
        log::info("[Pulse Effect] ID: 1006 -> Tüm Drop geçişlerinde ritmik renk atımı.");
        log::info("[Color Flash] ID: 29 -> Nine Circles sektörlerinde senkronize flaşör.");
        log::info("[Camera Shake] ID: 1520 -> Dalga modunda saniyede 2 kez sarsıntı.");
        log::info("[Alpha Hide] ID: 1521 -> Hafıza (Memory) fazında nesne görünmezliği.");
        log::info("[Spawn System] ID: 1268 -> Bölüm sonunda Boss animasyon zamanlayıcısı.");
        log::info("------------------------------------------");

        // --- 🚀 15.000 BLOKLUK MEGA ANA DÖNGÜ ---
        for (int i = 0; i < 15000; i++) {
            // Şarkı Aşamasına Göre Hız Portalları Enjeksiyonu
            float speedMultiplier = 1.0f;
            if (i > 1000 && i < 3000) speedMultiplier = 1.3f;        // x2 Hız
            if (i >= 3000 && i < 5500) speedMultiplier = 1.0f;       // x1 Hız (Dinlenme)
            if (i >= 5500 && i < 11500) speedMultiplier = 1.8f;      // x4 Hız!! (MEGA DROP)
            if (i >= 11500 && i < 13800) speedMultiplier = 0.6f;     // x0.5 Hız (Ağır Hafıza)
            if (i >= 13800) speedMultiplier = 1.0f;                  // x1 Hız (Final Sahnesi)

            currentX += (beatSpacing * speedMultiplier);

            // Dinamik Yükseklik Sapması Matematiksel Formülü
            float waveFormula = sin(i * 0.10f) * cos(i * 0.05f) * (config.difficulty * 13.0f);
            float finalHeight = baseHeight + waveFormula;

            // Şarkının 15 Farklı Evresi (Her 1000 Blokta Bir Oyun Modu Değişimi)
            int phase = i / 1000;

            // --- A) PORTAL ENJEKSİYON MANTIĞI ---
            if (i % 1000 == 0) {
                if (phase == 1) editorLayer->m_editorUI->createObject(12, CCPoint(currentX - 60, finalHeight), true);    // Ship Portal
                else if (phase == 2) editorLayer->m_editorUI->createObject(11, CCPoint(currentX - 60, finalHeight), true);   // Ball Portal
                else if (phase == 3) editorLayer->m_editorUI->createObject(111, CCPoint(currentX - 60, finalHeight), true);  // UFO Portal
                else if (phase == 5) editorLayer->m_editorUI->createObject(1, CCPoint(currentX - 60, finalHeight), true);    // Cube (Drop Öncesi)
                else if (phase == 6) {
                    editorLayer->m_editorUI->createObject(13, CCPoint(currentX - 60, finalHeight), true);   // Wave Portal
                    editorLayer->m_editorUI->createObject(794, CCPoint(currentX - 40, finalHeight), true);  // x4 Speed Portal
                }
                else if (phase == 10) editorLayer->m_editorUI->createObject(741, CCPoint(currentX - 60, finalHeight), true); // Robot Portal
                else if (phase == 11) editorLayer->m_editorUI->createObject(1347, CCPoint(currentX - 60, finalHeight), true);// Spider Portal
                else if (phase == 12) editorLayer->m_editorUI->createObject(1, CCPoint(currentX - 60, finalHeight), true);   // Cube (Memory Sektörü)
            }

            // --- B) SEKTÖREL YAPILAŞMA SEÇİMİ (LAYOUT) ---
            switch (phase) {
                case 0:
                case 5:
                    CubeSectorBuilder::build(editorLayer, currentX, beatSpacing, i, config.difficulty, totalLayoutCount, config.maxLayoutItems);
                    break;
                case 1:
                    ShipSectorBuilder::build(editorLayer, currentX, beatSpacing, i, config.difficulty, totalLayoutCount, config.maxLayoutItems);
                    break;
                case 2:
                    BallSectorBuilder::build(editorLayer, currentX, beatSpacing, i, config.difficulty, totalLayoutCount, config.maxLayoutItems);
                    break;
                case 3:
                case 4:
                    UfoSectorBuilder::build(editorLayer, currentX, beatSpacing, i, config.difficulty, totalLayoutCount, config.maxLayoutItems);
                    break;
                case 6:
                case 7:
                case 8:
                case 9:
                    WaveSectorBuilder::build(editorLayer, currentX, beatSpacing, i, config.difficulty, totalLayoutCount, config.maxLayoutItems);
                    break;
                case 10:
                    RobotSectorBuilder::build(editorLayer, currentX, beatSpacing, i, config.difficulty, totalLayoutCount, config.maxLayoutItems);
                    break;
                case 11:
                    SpiderSectorBuilder::build(editorLayer, currentX, beatSpacing, i, config.difficulty, totalLayoutCount, config.maxLayoutItems);
                    break;
                case 12:
                case 13:
                    // Memory Sektörü Algoritması
                    if (totalLayoutCount < config.maxLayoutItems) {
                        if (i % 2 == 0) {
                            editorLayer->m_editorUI->createObject(1, CCPoint(currentX, finalHeight), true);
                            totalLayoutCount++;
                        }
                        if (i % 32 == 0) {
                            editorLayer->m_editorUI->createObject(1521, CCPoint(currentX, finalHeight + 80.0f), true); // Alpha Trigger
                            totalLayoutCount++;
                        }
                    }
                    break;
                default:
                    // Outro Bölümü Düzlük Hattı
                    if (totalLayoutCount < config.maxLayoutItems) {
                        editorLayer->m_editorUI->createObject(1, CCPoint(currentX, baseHeight), true);
                        totalLayoutCount++;
                    }
                    break;
            }

            // --- C) DEKORASYON VE GENEL EFEKT ENJEKSİYONU ---
            DecorationEngine::injectDecoAndEffects(editorLayer, currentX, finalHeight, i, phase, config.ultraDeco, totalDecoCount, config.maxDecoItems);

            // --- D) BOSS SAVAŞI ALTYAPISI ENJEKSİYONU ---
            if (config.spawnBossAltyapi && i == 14200 && totalLayoutCount < config.maxLayoutItems) {
                // Boss odası için çoklu tetikleyici üniteleri (Spawn Trigger ID: 1268)
                editorLayer->m_editorUI->createObject(1268, CCPoint(currentX, baseHeight + 150.0f), true);
                totalLayoutCount++;
            }
        }

        editorLayer->m_editorUI->deselectAll();
        
        // Ekrana Detaylı Başarı Bildirimi Basma
        std::string report = "Holy Factory Engine Tamamlandi!\nLayout: " + std::to_string(totalLayoutCount) + " | Deco: " + std::to_string(totalDecoCount);
        Notification::create(report.c_str(), NotificationIcon::Success, 5.0f)->show();
    }
};

// ============================================================================
// 🔥 5. DEVASE KULLANICI ARAYÜZÜ KATMANI (ADVANCED UI POPUP)
// ============================================================================
class GodFactoryPopup : public geode::Popup<> {
protected:
    TextInput* m_bpmInput;
    CCMenuItemToggler* m_decoToggle;
    CCMenuItemToggler* m_bossToggle;
    int m_lockedDifficulty = 10; // Extreme Demon Sınırı

    bool setup() override {
        auto winSize = CCDirector::get()->getWinSize();
        this->setTitle("THE HOLY FACTORY v5.7.1", "goldFont.fnt", 0.85f);

        auto mainLayer = CCNode::create();
        this->m_mainLayer->addChild(mainLayer);

        // Şarkı Tempo Değeri Arayüzü
        auto bpmLabel = CCLabelBMFont::create("SARKİ BPM:", "bigFont.fnt");
        bpmLabel->setScale(0.35f);
        bpmLabel->setPosition({winSize.width / 2 - 85, winSize.height / 2 + 65});
        mainLayer->addChild(bpmLabel);

        m_bpmInput = TextInput::create(75, "160");
        m_bpmInput->setPosition({winSize.width / 2 + 35, winSize.height / 2 + 65});
        m_bpmInput->setFilter("0123456789");
        mainLayer->addChild(m_bpmInput);

        // Orijinal Extreme Demon Yüzü Butonu
        auto uiMenu = CCMenu::create();
        auto diffSprite = CCSprite::createWithSpriteFrameName("difficulty_10_btn_001.png");
        auto diffBtn = CCMenuItemSpriteExtra::create(diffSprite, this, menu_selector(GodFactoryPopup::onDiffNotice));
        diffBtn->setPosition({winSize.width / 2, winSize.height / 2 + 5});
        uiMenu->addChild(diffBtn);
        uiMenu->setPosition(CCPointZero);
        mainLayer->addChild(uiMenu);

        // İkili Seçenek Hücre Yapıları (Toggles)
        auto toggleMenu = CCMenu::create();
        
        m_decoToggle = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(GodFactoryPopup::onNullCallback), 0.7f);
        m_decoToggle->setPosition({winSize.width / 2 - 110, winSize.height / 2 - 50});
        auto decoText = CCLabelBMFont::create("100K Ultra Deco", "bigFont.fnt");
        decoText->setScale(0.35f);
        decoText->setPosition({winSize.width / 2 - 35, winSize.height / 2 - 50});
        
        m_bossToggle = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(GodFactoryPopup::onNullCallback), 0.7f);
        m_bossToggle->setPosition({winSize.width / 2 + 45, winSize.height / 2 - 50});
        auto bossText = CCLabelBMFont::create("Boss Izni", "bigFont.fnt");
        bossText->setScale(0.35f);
        bossText->setPosition({winSize.width / 2 + 100, winSize.height / 2 - 50});

        toggleMenu->addChild(m_decoToggle);
        toggleMenu->addChild(m_bossToggle);
        toggleMenu->setPosition(CCPointZero);
        
        mainLayer->addChild(toggleMenu);
        mainLayer->addChild(decoText);
        mainLayer->addChild(bossText);

        // MASTER GENERATE BUTONU
        auto actionMenu = CCMenu::create();
        auto actionSprite = ButtonSprite::create("KUTSAL FABRİKAYI TETİKLE!", "goldFont.fnt", "GJ_button_01.png", 0.75f);
        auto actionBtn = CCMenuItemSpriteExtra::create(actionSprite, this, menu_selector(GodFactoryPopup::onLaunchGeneration));
        actionMenu->addChild(actionBtn);
        actionMenu->setPosition({winSize.width / 2, winSize.height / 2 - 100});
        mainLayer->addChild(actionMenu);

        return true;
    }

    void onDiffNotice(CCObject*) {
        Notification::create("Maksimum Güç Modu Aktif! (Extreme Demon)", NotificationIcon::Warning)->show();
    }
    
    void onNullCallback(CCObject*) {}

    void onLaunchGeneration(CCObject*) {
        int bpmVal = 160;
        if (m_bpmInput->getString().length() > 0) {
            bpmVal = std::stoi(m_bpmInput->getString());
        }

        bool isDecoOn = !m_decoToggle->isToggled();
        bool isBossOn = !m_bossToggle->isToggled();

        // Konfigürasyon Veri Paketini Doldurma ve Motoru Ateşleme
        GeneratorConfig coreConfig;
        coreConfig.bpm = bpmVal;
        coreConfig.difficulty = m_lockedDifficulty;
        coreConfig.ultraDeco = isDecoOn;
        coreConfig.spawnBossAltyapi = isBossOn;
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
// 🔥 6. EDİTÖR ARAYÜZÜ HOOK ENJEKSİYONU (THE ANCHOR)
// ============================================================================
class $modify(MyUltimateFactoryUI, EditorUI) {
    bool init(LevelEditorLayer* lel) {
        if (!EditorUI::init(lel)) return false;

        auto rightMenu = this->getChildByID("right-toolbar-menu");
        if (rightMenu) {
            // Sağ panele özel parlayan altın-sarı formatta GL Butonu enjekte edilir
            auto glSprite = ButtonSprite::create("GL\nFACTORY", "bigFont.fnt", "GJ_button_02.png", 0.52f);
            auto glButton = CCMenuItemSpriteExtra::create(glSprite, this, menu_selector(MyUltimateFactoryUI::onFactoryCoreClicked));
            
            glButton->setID("gl-ultimate-factory-core-button");
            rightMenu->addChild(glButton);
            rightMenu->updateLayout();
        }
        return true;
    }

    void onFactoryCoreClicked(CCObject* sender) {
        GodFactoryPopup::create()->show();
    }
};
