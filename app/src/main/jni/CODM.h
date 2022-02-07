#include "Includes.h"
#include "Tools.h"
#include "Engine/Canvas.h"
#include "fake_dlfcn.h"
#include "Il2Cpp.h"
#include "Vector2.hpp"
#include "Vector3.hpp"
#include "Quaternion.hpp"
//extern void StartRuntimeHook(const char *);
// ================================================================================================================================ //
#define SLEEP_TIME 1000LL / 120LL

std::map<std::string, u_long> Config;

int g_screenWidth = 0, g_screenHeight = 0;
bool bInitDone = false;
int screenWidth = 0, screenHeight = 0;
uintptr_t g_il2cpp;
// ================================================================================================================================ //
std::map<std::string, uintptr_t> Fields;
std::map<std::string, uintptr_t> Methods;


// ================================================================================================================================ //
class Transform {
public:
    Vector3 get_position() {
        auto Transform_get_position = (Vector3 (*)(Transform *)) (Methods["Transform::get_position"]);
        return Transform_get_position(this);
    }
};

class Component {
public:
    Transform *get_transform() {
        auto Component_get_transform = (Transform *(*)(Component *)) (Methods["Component::get_transform"]);
        return Component_get_transform(this);
    }
};

class Camera {
public:
    static Camera *get_main() {
        auto Camera_get_main = (Camera *(*)()) (Methods["Camera::get_main"]);
        return Camera_get_main();
    }
};

Vector3 WorldToScreen(Vector3 pos) {
    auto main = Camera::get_main();
    if (main) {
        auto Camera_WorldToScreenPoint = (Vector3 (*)(Camera *, Vector3)) (Methods["Camera::WorldToScreenPoint"]);
        return Camera_WorldToScreenPoint(main, pos);
    }
    return {0, 0, 0};
}

uintptr_t GetClosestTarget() {
    uintptr_t result = 0;

    float MaxDist = std::numeric_limits<float>::infinity();

    void *BaseWorld_Instance = 0;
    Il2CppGetStaticFieldValue("Assembly-CSharp.dll", "GameEngine", "BaseWorld", "Instance", &BaseWorld_Instance);
    if (BaseWorld_Instance) {
        auto m_Game = *(uintptr_t *) ((uintptr_t) BaseWorld_Instance + Fields["BaseWorld::m_Game"]);
        if (m_Game) {
            auto Gameplay_get_LocalPawn = (uintptr_t (*)()) (Methods["Gameplay::get_LocalPawn"]);
            auto LocalPawn = Gameplay_get_LocalPawn();
            if (LocalPawn) {
                Vector3 MyPos{0, 0, 0};

                auto local_m_Mesh = *(Transform **) (LocalPawn + Fields["Pawn::m_Mesh"]);
                if (local_m_Mesh) {
                    MyPos = local_m_Mesh->get_position();
                }

                auto EnemyPawns = *(List<uintptr_t> **) (m_Game + Fields["BaseGame::EnemyPawns"]);
                if (EnemyPawns) {
                    auto Items = EnemyPawns->getItems();
                    if (Items) {
                        for (int i = 0; i < EnemyPawns->getSize(); i++) {
                            auto Pawn = Items[i];
                            if (Pawn) {
                                if (!*(bool *) (Pawn + Fields["Pawn::m_IsAlive"]))
                                    continue;

                                auto m_Mesh = *(Transform **) (Pawn + Fields["Pawn::m_Mesh"]);
                                if (!m_Mesh)
                                    continue;

                                auto RootPos = m_Mesh->get_position();
                                float Distance = Vector3::Distance(MyPos, RootPos);

                                if (Distance < MaxDist) {
                                    result = Pawn;
                                    MaxDist = Distance;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return result;
}

bool isInsideFOV(int x, int y) {
    if (!Config["AIM::SIZE"])
        return true;

    int circle_x = g_screenWidth / 2;
    int circle_y = g_screenHeight / 2;
    int rad = Config["AIM::SIZE"];
    return (x - circle_x) * (x - circle_x) + (y - circle_y) * (y - circle_y) <= rad * rad;
}

uintptr_t GetInsideFOVTarget() {
    uintptr_t result = 0;

    float MaxDist = std::numeric_limits<float>::infinity();

    void *BaseWorld_Instance = 0;
    Il2CppGetStaticFieldValue("Assembly-CSharp.dll", "GameEngine", "BaseWorld", "Instance", &BaseWorld_Instance);
    if (BaseWorld_Instance) {
        auto m_Game = *(uintptr_t *) ((uintptr_t) BaseWorld_Instance + Fields["BaseWorld::m_Game"]);
        if (m_Game) {
            auto Gameplay_get_LocalPawn = (uintptr_t (*)()) (Methods["Gameplay::get_LocalPawn"]);
            auto LocalPawn = Gameplay_get_LocalPawn();
            if (LocalPawn) {
                Vector3 MyPos{0, 0, 0};

                auto local_m_Mesh = *(Transform **) (LocalPawn + Fields["Pawn::m_Mesh"]);
                if (local_m_Mesh) {
                    MyPos = local_m_Mesh->get_position();
                }

                auto EnemyPawns = *(List<uintptr_t> **) (m_Game + Fields["BaseGame::EnemyPawns"]);
                if (EnemyPawns) {
                    auto Items = EnemyPawns->getItems();
                    if (Items) {
                        for (int i = 0; i < EnemyPawns->getSize(); i++) {
                            auto Pawn = Items[i];
                            if (Pawn) {
                                if (!*(bool *) (Pawn + Fields["Pawn::m_IsAlive"]))
                                    continue;

                                auto m_HeadBone = *(Transform **) (Pawn + Fields["Pawn::m_HeadBone"]);
                                if (!m_HeadBone)
                                    continue;

                                auto HeadSc = WorldToScreen(m_HeadBone->get_position());

                                Vector2 v2Middle = Vector2((float) (g_screenWidth / 2), (float) (g_screenHeight / 2));
                                Vector2 v2Loc = Vector2(HeadSc.X, HeadSc.Y);

                                if (isInsideFOV((int) HeadSc.X, (int) HeadSc.Y)) {
                                    float Distance = Vector2::Distance(v2Middle, v2Loc);

                                    if (Distance < MaxDist) {
                                        result = Pawn;
                                        MaxDist = Distance;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return result;
}
// ================================================================================================================================ //

// ================================================================================================================================ //
void native_onCanvasDraw(JNIEnv *env, jobject obj, jobject canvas, int screenWidth, int screenHeight, float screenDensity) {
    static Canvas *m_Canvas = 0;
    if (!m_Canvas) {
        LOGI("Canvas Object: %p | Width: %d | Height: %d | Density: %f", canvas, screenWidth, screenHeight, screenDensity);
        m_Canvas = new Canvas(env, screenWidth, screenHeight, screenDensity);
    }

    m_Canvas->UpdateCanvas(canvas);

    g_screenWidth = screenWidth;
    g_screenHeight = screenHeight;
	
    float lineSize = m_Canvas->scaleSize(0.8f);
	
    if (!bInitDone)
        return;

    auto Screen_SetResolution = (void (*)(void *, int, int, bool)) (Methods["Screen::SetResolution"]);
    Screen_SetResolution(0, screenWidth, screenHeight, true);

    void *BaseWorld_Instance = 0;
    Il2CppGetStaticFieldValue("Assembly-CSharp.dll", "GameEngine", "BaseWorld", "Instance", &BaseWorld_Instance);
    if (Tools::IsPtrValid(BaseWorld_Instance)) {
        auto m_Game = *(uintptr_t *) ((uintptr_t) BaseWorld_Instance + Fields["BaseWorld::m_Game"]);
        if (Tools::IsPtrValid((void *) m_Game)) {
            auto Gameplay_get_LocalPawn = (uintptr_t (*)()) (Methods["Gameplay::get_LocalPawn"]);
            auto LocalPawn = Gameplay_get_LocalPawn();
              if (Tools::IsPtrValid((void *) LocalPawn)) {
                if (Config["AIM::TARGET"] == 1) {
                    m_Canvas->drawCircle(screenWidth / 2, screenHeight / 2, Config["AIM::SIZE"], 1.5f, false, ARGB(255, 0, 255, 0));
                }

                Vector3 MyPos{0, 0, 0};

                auto local_m_Mesh = *(Transform **) (LocalPawn + Fields["Pawn::m_Mesh"]);
                if (Tools::IsPtrValid(local_m_Mesh)) {
                    MyPos = local_m_Mesh->get_position();
                }

                auto EnemyPawns = *(List<uintptr_t> **) (m_Game + Fields["BaseGame::EnemyPawns"]);
                if (Tools::IsPtrValid(EnemyPawns)) {
                    auto Items = EnemyPawns->getItems();
                    if (Tools::IsPtrValid(Items)) {
                        for (int i = 0; i < EnemyPawns->getSize(); i++) {
                            auto Pawn = Items[i];
                            if (Tools::IsPtrValid((void *) Pawn)) {
                                auto m_PlayerInfo = *(uintptr_t *) (Pawn + Fields["Pawn::m_PlayerInfo"]);
                                if (Tools::IsPtrValid((void *) m_PlayerInfo)) {
                                    auto m_HeadBone = *(Transform **) (Pawn + Fields["Pawn::m_HeadBone"]);
                                    if (!Tools::IsPtrValid(m_HeadBone))
                                        continue;

                                    auto m_Mesh = *(Transform **) (Pawn + Fields["Pawn::m_Mesh"]);
                                    if (!m_Mesh)
                                        continue;

                                    auto HeadPos = m_HeadBone->get_position();
                                    auto HeadSc = WorldToScreen(HeadPos);                                 
                                    auto RootPos = m_Mesh->get_position();
                                    auto RootSc = WorldToScreen(RootPos);

                                    float Distance = Vector3::Distance(MyPos, RootPos);
                                      if (HeadSc.Z > 0) {
                                      if (Config["ESP::LINE"]) {
                                        m_Canvas->drawLine(screenWidth / 2, 0, HeadSc.X, screenHeight - HeadSc.Y, lineSize, RED);
                                       }					                                        
                                        if (Config["ESP::BOX"]) {
                                            float boxHeight = abs(HeadSc.Y - RootSc.Y);
                                            float boxWidth = boxHeight * 0.65f;
                                            Vector2 vBox = {HeadSc.X - (boxWidth / 2), HeadSc.Y};

                                            m_Canvas->drawBorder(vBox.X, screenHeight - vBox.Y, boxWidth, boxHeight, 1.5f, RED);
                                        }
                                        if (Config["ESP::HEALTH"]) {
                                            auto m_AttackableInfo = *(uintptr_t *) (Pawn + Fields["AttackableTarget::m_AttackableInfo"]);
                                            if (m_AttackableInfo) {
                                                int CurHP = (int) *(float *) (m_AttackableInfo + Fields["AttackableTargetInfo::m_Health"]);
                                                int MaxHP = (int) *(float *) (m_AttackableInfo + Fields["AttackableTargetInfo::m_MaxHealth"]);

                                                long Color = ARGB(155, std::min(((510 * (MaxHP - CurHP)) / MaxHP), 255), std::min(((510 * CurHP) / MaxHP), 255), 0);

                                                auto AboveHead = HeadPos;
                                                Vector3 AboveHeadSc = WorldToScreen(AboveHead);
                                                if (AboveHeadSc.Z > 0) {
                                                    auto mWidth = m_Canvas->scaleSize(35.f);
                                                    auto mHeight = mWidth * 0.175f;

                                                    AboveHeadSc.X -= (mWidth / 2);
                                                    AboveHeadSc.Y += (mHeight * 2);

                                                    m_Canvas->drawBox(AboveHeadSc.X, screenHeight - AboveHeadSc.Y, (CurHP * mWidth / MaxHP), mHeight, Color);
                                                    m_Canvas->drawBorder(AboveHeadSc.X, screenHeight - AboveHeadSc.Y, mWidth, mHeight, 1.0f, BLACK);
                                                }
                                            }
                                        }
                                        if (Config["ESP::NAME"] || Config["ESP::DISTANCE"]) {
                                            Vector3 BelowRoot = RootPos;
                                            Vector3 BelowRootSc = WorldToScreen(BelowRoot);
                                            if (BelowRootSc.Z > 0) {
                                                std::wstring ws;

                                                if (Config["ESP::NAME"]) {
                                                    auto m_NickName = *(String **) (m_PlayerInfo + Fields["PlayerInfo::m_NickName"]);
                                                    if (m_NickName) {
                                                        ws += m_NickName->WCString();
                                                    }

                                                    if (Config["ESP::DISTANCE"]) {
                                                        if (!ws.empty())
                                                            ws += L" [";
                                                        ws += std::to_wstring((int) Distance);
                                                        ws += L"m]";
                                                    }

                                                    auto mText = m_Canvas->getTextBounds(ws.c_str(), 0, ws.size());
                                                    m_Canvas->drawText(ws.c_str(), BelowRootSc.X, screenHeight - BelowRootSc.Y + mText->getHeight(), 7.5f, Align::CENTER,PINK, BLACK);
													}                                        
											}
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
void *Main_Thread(void *) {
    while (g_il2cpp) {
        auto t1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        void *BaseWorld_Instance = 0;
        Il2CppGetStaticFieldValue("Assembly-CSharp.dll", "GameEngine", "BaseWorld", "Instance", &BaseWorld_Instance);
        if (BaseWorld_Instance) {
            auto m_Game = *(uintptr_t *) ((uintptr_t) BaseWorld_Instance + Fields["BaseWorld::m_Game"]);
            if (m_Game) {
                auto Gameplay_get_LocalPawn = (uintptr_t (*)()) (Methods["Gameplay::get_LocalPawn"]);
                auto LocalPawn = Gameplay_get_LocalPawn();
                if (LocalPawn) {
                    if (Config["AIM::AIMBOT"]) {
                        bool bTriggerReady = Config["AIM::TRIGGER"] == 0;
                        if (Config["AIM::TRIGGER"] == 1) {
                            auto Pawn_get_IsFiring = (bool (*)(uintptr_t)) (Methods["Pawn::get_IsFiring"]);
                            bTriggerReady = Pawn_get_IsFiring(LocalPawn);
                        } else if (Config["AIM::TRIGGER"] == 2) {
                            auto Pawn_IsAiming = (bool (*)(uintptr_t)) (Methods["Pawn::IsAiming"]);
                            bTriggerReady = Pawn_IsAiming(LocalPawn);
                        }
                        if (bTriggerReady) {
                            uintptr_t Target = 0;
                            if (Config["AIM::TARGET"] == 0) {
                                Target = GetClosestTarget();
                            }
                            if (Config["AIM::TARGET"] == 1) {
                                Target = GetInsideFOVTarget();
                            }
                            if (Target) {
                                Vector3 targetPos;
                                if (Config["AIM::LOCATION"] == 0) {
                                   auto m_HeadBone = *(Transform **) (Target + Fields["Pawn::m_HeadBone"]);
                                if (!m_HeadBone)
                                    continue; 

                                targetPos = m_HeadBone->get_position();
                            }
                                if (Config["AIM::LOCATION"] == 1) {
                                   auto m_HeadBone = *(Transform **) (Target + Fields["Pawn::m_HeadBone"]);
                                if (!m_HeadBone)
                                    continue;

                                targetPos = m_HeadBone->get_position();
                                targetPos.Y -= 0.2f;
                            }
                                if (Config["AIM::LOCATION"] == 2) {
                                   auto m_HeadBone = *(Transform **) (Target + Fields["Pawn::m_HeadBone"]);
                                if (!m_HeadBone)
                                    continue;

                                targetPos = m_HeadBone->get_position();
                                targetPos.Y -= 0.4f;
                            }

                                auto main = Camera::get_main();
                                if (main) {
                                    auto mainView = ((Component *) main)->get_transform();
                                    if (mainView) {
                                        auto Pawn_SetAimRotation = (void (*)(uintptr_t, Quaternion)) (Methods["Pawn::SetAimRotation"]);
                                        Pawn_SetAimRotation(LocalPawn, Quaternion::LookRotation(targetPos - mainView->get_position(), Vector3::Up()));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        auto td = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - t1;
        std::this_thread::sleep_for(std::chrono::milliseconds(std::max(std::min(0LL, SLEEP_TIME - td), SLEEP_TIME)));
    }
    return 0;
}
// ================================================================================================================================ //

void AutoAdjustResolution(uintptr_t thiz) {
    return;
}

// ================================================================================================================================ //
void *Init_Thread(void *) {
    while (!g_il2cpp) {
        g_il2cpp = Tools::GetBaseAddress("libil2cpp.so");
        sleep(1);
    }

    LOGI("libil2cpp.so: %p", g_il2cpp);

    Il2CppAttach();

    sleep(5);

    Methods["Transform::get_position"] = (uintptr_t) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Transform", "get_position");
    
    Methods["Camera::get_main"] = (uintptr_t) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Camera", "get_main");
    
    Methods["Camera::WorldToScreenPoint"] = (uintptr_t) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Camera", "WorldToScreenPoint", 1);
    
    Methods["Component::get_transform"] = (uintptr_t) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Component", "get_transform");
    
   Methods["Screen::SetResolution"] = (uintptr_t) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Screen", "SetResolution", 3);
    
    Methods["Gameplay::get_LocalPawn"] = (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "GameEngine", "GamePlay", "get_LocalPawn");

    Fields["BaseWorld::m_Game"] = Il2CppGetFieldOffset("Assembly-CSharp.dll", "GameEngine", "BaseWorld", "m_Game");
    Fields["BaseGame::AllPawns"] = Il2CppGetFieldOffset("Assembly-CSharp.dll", "GameBase", "BaseGame", "AllPawns");
    Fields["BaseGame::EnemyPawns"] = Il2CppGetFieldOffset("Assembly-CSharp.dll", "GameBase", "BaseGame", "EnemyPawns");

    Methods["Pawn::SetAimRotation"] = (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "GameBase", "Pawn", "SetAimRotation", 1);
    Methods["Pawn::get_IsFiring"] = (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "GameBase", "Pawn", "get_IsFiring");
    Methods["Pawn::IsAiming"] = (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "GameBase", "Pawn", "IsAiming");

    Fields["Pawn::m_PlayerInfo"] = Il2CppGetFieldOffset("Assembly-CSharp.dll", "GameBase", "Pawn", "m_PlayerInfo");
    Fields["Pawn::get_HeadBone"] = Il2CppGetFieldOffset("Assembly-CSharp.dll", "GameBase", "Pawn", "get_HeadBone");
    Fields["Pawn::m_HeadBone"] = Il2CppGetFieldOffset("Assembly-CSharp.dll", "GameBase", "Pawn", "m_HeadBone");
    Fields["Pawn::m_Mesh"] = Il2CppGetFieldOffset("Assembly-CSharp.dll", "GameBase", "Pawn", "m_Mesh");
    Fields["Pawn::m_IsAlive"] = Il2CppGetFieldOffset("Assembly-CSharp.dll", "GameBase", "Pawn", "m_IsAlive");

    Fields["AttackableTarget::m_AttackableInfo"] = Il2CppGetFieldOffset("Assembly-CSharp.dll", "GameEngine", "AttackableTarget", "m_AttackableInfo");
    Fields["AttackableTargetInfo::m_Health"] = Il2CppGetFieldOffset("Assembly-CSharp.dll", "GameEngine", "AttackableTargetInfo", "m_Health");
    Fields["AttackableTargetInfo::m_MaxHealth"] = Il2CppGetFieldOffset("Assembly-CSharp.dll", "GameEngine", "AttackableTargetInfo", "m_MaxHealth");

    Fields["PlayerInfo::m_NickName"] = Il2CppGetFieldOffset("Assembly-CSharp.dll", "GameEngine", "PlayerInfo", "m_NickName");


    bInitDone = true;

    return 0;
}

void native_Init(JNIEnv *env, jclass clazz, jobject mContext) {
    pthread_t t;
    pthread_create(&t, 0, Init_Thread, 0);
}

