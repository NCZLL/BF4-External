#include "overlay.h"
#include "ImGui\customed.h"
#include <string>

extern Memory m;
extern RECT GameSize;
extern POINT GamePos;

// Menu & Cheat
extern bool m_esp;
extern bool SwayEditor;
extern float SwayV;
extern bool UnlockAll;
extern bool Run;

bool m_TeamESP = false;
bool m_box = true;
bool m_line = false;
bool m_distance = true;
int m_maxdist = 650;
bool m_healthbar = true;
bool Alive = false;
float BoxHeight = 0.f;
const char* BoxList[] = { "2D", "3D" };
int m_boxstyle = 0;
Overlay v;

struct AxisAlignedBox
{
    Vector4 Min;
    Vector4 Max;
};

// ESP Color
ImVec4 color_Normal = { 1.f, 0.f, 0.f, 1.f };
ImVec4 color_Visible = { 0.f, 1.f, 0.f, 1.f };
ImVec4 color_Team = { 0.f, 1.f, 1.f, 1.f };

float GetDistance(Vector3 value1, Vector3 value2);
bool W2S(const Vector3& WorldPos, Vector3& ScreenPos);
void DrawBox(AxisAlignedBox aabb, Vector3 m_Position, float Yaw, ImColor color);
AxisAlignedBox AABB(DWORD_PTR soldier);

void Overlay::Info()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(GameSize.right , GameSize.bottom));
    ImGui::Begin("##BF4-Info", (bool*)NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs);

    ImGui::Text("[ unknowncheats.me ] BF4 Simple ESP");

    ImGui::End();
}

void Overlay::Menu()
{
    ImGui::Begin("Battlefield 4 FREE HACK - unknowncheats.me", (bool*)NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

    // Select Menu
    ImGui::BeginChild("##M1", ImVec2(250, 500 - 16));
    ImGui::SetCursorPos(ImVec2(16.f, 16.f));
    ImGui::BeginChild("##M2", ImVec2(250, 300), (bool*)false, ImGuiWindowFlags_NoBackground);

    static int Mselect = 0;
    if (ImGui::Button("Visual", ImVec2(220, 45)))
        Mselect = 0;
    else if (ImGui::Button("Misc", ImVec2(220, 45)))
        Mselect = 1;
    else if (ImGui::Button("Dev", ImVec2(220, 45)))
        Mselect = 2;

    ImGui::EndChild();
    ImGui::EndChild();

    ImGui::SameLine();

    // MainMenu
    ImGui::BeginChild("##M3", ImVec2(300, 500 - 16));

    ImGui::SetCursorPos(ImVec2(8.f, 8.f));
    ImGui::BeginChild("##M4", ImVec2(300.f - 16.f, 500 - 32), (bool*)false, ImGuiWindowFlags_NoBackground);

    ImGui::Text(" [ Main ]");
    ImGui::Separator();

    ImGui::NewLine();

    switch (Mselect)
    {
    case 0:
        ImGui::Text("[+] ESP");
        ImGui::Toggle("ESP", &m_esp, m_esp);
        ImGui::Toggle("Team ESP", &m_TeamESP, m_TeamESP);

        ImGui::NewLine();
        ImGui::Text("[+] ESP Option");
        ImGui::Checkbox("Box", &m_box);
        ImGui::Checkbox("Line", &m_line);
        ImGui::Checkbox("HealthBar", &m_healthbar);
        ImGui::Checkbox("Distance", &m_distance);
        break;
    case 1:
        ImGui::Text("[+] Recoil/Spread");
        ImGui::Checkbox("SwayEditor", &SwayEditor);
        ImGui::SliderFloat("WriteValue", &SwayV, 0.f, 1.f);
        ImGui::NewLine();
        ImGui::Text("[+] MISC");
        ImGui::Checkbox("UnlockAll", &UnlockAll);
        break;
    case 2:
        ImGui::Text("[+] ProcessInfo");
        ImGui::Text("Process ID  : %d", m.PID);
        ImGui::Text("  pHandle   : 0x%lx\n", m.pHandle);

        ImGui::NewLine();
        ImGui::Text("[+] WindowInfo");
        ImGui::Text(" ClassName  : %s", m.ClassName);
        ImGui::Text("WindowName  : %s", m.MenuName);
        ImGui::Text(" GameSize   : %d, %d", GameSize.right, GameSize.bottom);

        ImGui::NewLine();

        // White button
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(1.f, 1.f, 1.f, 0.35f));
        if (ImGui::Button("EXIT"))
            Run = false;
        ImGui::PopStyleColor();
        break;
    default:
        break;
    }

    ImGui::EndChild();
    ImGui::EndChild();

    ImGui::SameLine();

    if (Mselect == 0)
    {
        ImGui::BeginChild("##M5", ImVec2(300, 500 - 16));

        ImGui::SetCursorPos(ImVec2(8.f, 8.f));
        ImGui::BeginChild("##M6", ImVec2(300.f - 16.f, 500 - 32), (bool*)false, ImGuiWindowFlags_NoBackground);

        ImGui::Text(" [ Sub ]");
        ImGui::Separator();

        ImGui::Text("[+] ESP Options");
        ImGui::ColorEdit4("Normal", &color_Normal.x);
        ImGui::ColorEdit4("Visible", &color_Visible.x);
        ImGui::ColorEdit4("Team", &color_Team.x);

        ImGui::NewLine();

        ImGui::Text("[+] ESP Options #2");
        ImGui::SliderInt("Distance (m)", &m_maxdist, 50, 1000);
        ImGui::Combo("BoxStyle", &m_boxstyle, BoxList, IM_ARRAYSIZE(BoxList));

        ImGui::EndChild();
        ImGui::EndChild();
    }

    ImGui::End();
}

void Overlay::ESP()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(GameSize.right, GameSize.bottom));
    ImGui::Begin("##BF4-ESP", (bool*)NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs);
    
    DWORD_PTR ClientGameContext = m.RPM<DWORD_PTR>(OFFSET_CLIENTGAMECONTEXT);
    DWORD_PTR PlayerManager = m.RPM<DWORD_PTR>(ClientGameContext + OFFSET_PLAYERMANAGER);
    DWORD_PTR Player = m.RPM<DWORD_PTR>(PlayerManager + OFFSET_PLAYER);
    DWORD_PTR LocalPlayer = m.RPM<DWORD_PTR>(PlayerManager + OFFSET_LOCALPLAYER);
    DWORD_PTR LocalSoldier = m.RPM<DWORD_PTR>(LocalPlayer + OFFSET_SOLDIER);

    int LocalTeam = m.RPM<int>(LocalPlayer + OFFSET_TEAM);
    DWORD_PTR LHealthComponent = m.RPM<DWORD_PTR>(LocalSoldier + 0x140);
    float LocalHealth = m.RPM<float>(LHealthComponent + 0x20);
    DWORD_PTR LPosComponent = m.RPM<DWORD_PTR>(LocalSoldier + 0x490);
    Vector3 LocalPos = m.RPM<Vector3>(LPosComponent + 0x30);

    for (int i = 0; i < 64; i++)
    {
        if (LocalHealth == 0.f || LocalHealth < 0.f)
        {
            Alive = false;
            //continue;
        }
        else
        {
            Alive = true;
        }

        DWORD_PTR pPlayerEntity = m.RPM<DWORD_PTR>(Player + (i * 0x08));
        DWORD_PTR pPlayerSoldier = m.RPM<DWORD_PTR>(pPlayerEntity + OFFSET_SOLDIER);

        // Invalid Player
        if (pPlayerEntity == 0)
            continue;

        if (pPlayerEntity == LocalPlayer)
            continue;

        // Team
        int Team = m.RPM<int>(pPlayerEntity + OFFSET_TEAM);

        // Health
        DWORD_PTR HealthComponent = m.RPM<DWORD_PTR>(pPlayerSoldier + 0x140);
        float Health = m.RPM<float>(HealthComponent + 0x20);

        // Pos
        DWORD_PTR PosComponent = m.RPM<DWORD_PTR>(pPlayerSoldier + 0x490);
        Vector3 PlayerOrigin = m.RPM<Vector3>(PosComponent + 0x30);

        float distance = GetDistance(LocalPos, PlayerOrigin);

        if (m_maxdist < (int)distance)
            continue;

        // Visible
        bool Visible = m.RPM<bool>(pPlayerSoldier + 0x5B1);
        Visible = !Visible;

        // Box用
        float Yaw = m.RPM<float>(pPlayerSoldier + 0x4D8);

        // Some check
        if (Health <= 0.f)
            continue;
        else if (Team == LocalTeam && !m_TeamESP)
            continue;
        else if (PlayerOrigin.x == 0.f && PlayerOrigin.y == 0.f)
            continue;

        // W2S
        Vector3 pScreen = {};
        W2S(PlayerOrigin, pScreen);
        if (pScreen.x == 0 && pScreen.y == 0)
            continue;

        // ESP color
        ImColor color;

        if (Visible)
            color = color_Visible;
        else
            color = color_Normal;

        if (m_TeamESP)
        {
            if (Team == LocalTeam)
                color = color_Team;
        }

        // 3D Box
        DrawBox(AABB(pPlayerSoldier), PlayerOrigin, Yaw, color);

        // 2D Box
        float BoxWidth = BoxHeight / 4;
        if (m_box && m_boxstyle == 0)
        {
            Line(ImVec2(pScreen.x - BoxWidth, pScreen.y), ImVec2(pScreen.x + BoxWidth, pScreen.y), color, 1);
            Line(ImVec2(pScreen.x - BoxWidth, pScreen.y - BoxHeight), ImVec2(pScreen.x + BoxWidth, pScreen.y - BoxHeight), color, 1);
            Line(ImVec2(pScreen.x - BoxWidth, pScreen.y), ImVec2(pScreen.x - BoxWidth, pScreen.y - BoxHeight), color, 1);
            Line(ImVec2(pScreen.x + BoxWidth, pScreen.y), ImVec2(pScreen.x + BoxWidth, pScreen.y - BoxHeight), color, 1);
        }

        // Line
        if (m_line)
            Line(ImVec2(GameSize.right / 2, GameSize.bottom), ImVec2(pScreen.x, pScreen.y), color, 1);

        // Health Bar
        if (m_healthbar)
        {
            BaseBar(pScreen.x - BoxWidth - 5, pScreen.y + 1, 3, -BoxHeight - 1, 100);
            ProgressBar(pScreen.x - BoxWidth - 4, pScreen.y, 1, -BoxHeight, Health, 100);
        }

        // Distance
        if (m_distance)
        {
            // float to Text
            std::string text = std::to_string((int)distance) + "m";

            // draw box Centor
            ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
            float TextCentor = textSize.x / 2.f;

            String(ImVec2(pScreen.x - TextCentor, pScreen.y), ImColor(1.f, 1.f, 1.f, 1.f), text.c_str());
        }
    }

    ImGui::End();
}

bool W2S(const Vector3& WorldPos, Vector3& ScreenPos)
{
    DWORD_PTR GameRenderer = m.RPM<DWORD_PTR>(OFFSET_GAMERENDERER);
    DWORD_PTR RenderView = m.RPM<DWORD_PTR>(GameRenderer + 0x60);

    if (RenderView == 0)
        return false;

    DWORD_PTR DXRenderer = m.RPM<DWORD_PTR>(OFFSET_DXRENDERER);
    DWORD_PTR m_pScreen = m.RPM<DWORD_PTR>(DXRenderer + 0x38);

    if (m_pScreen == 0)
        return false;

    Matrix view_x_projection = m.RPM<Matrix>(RenderView + 0x420);

    int ScreenWidth = m.RPM<int>(m_pScreen + 0x58);
    int ScreenHeight = m.RPM<int>(m_pScreen + 0x5C);

    float cX = ScreenWidth * 0.5f;
    float cY = ScreenHeight * 0.5f;

    float w = view_x_projection(0, 3) * WorldPos.x + view_x_projection(1, 3) * WorldPos.y + view_x_projection(2, 3) * WorldPos.z + view_x_projection(3, 3);

    if (w < 0.65f)
    {
        ScreenPos.z = w;
        return false;
    }

    float x = view_x_projection(0, 0) * WorldPos.x + view_x_projection(1, 0) * WorldPos.y + view_x_projection(2, 0) * WorldPos.z + view_x_projection(3, 0);
    float y = view_x_projection(0, 1) * WorldPos.x + view_x_projection(1, 1) * WorldPos.y + view_x_projection(2, 1) * WorldPos.z + view_x_projection(3, 1);

    ScreenPos.x = cX + cX * x / w;
    ScreenPos.y = cY - cY * y / w;
    ScreenPos.z = w;

    return true;
}

AxisAlignedBox AABB(DWORD_PTR soldier)
{
    AxisAlignedBox aabb = {};
    int Pose = m.RPM<int>(soldier + 0x4F0);

    if (Pose == 0) // standing
    {
        aabb.Min = Vector4(-0.350000f, 0.000000f, -0.350000f, 0);
        aabb.Max = Vector4(0.350000f, 1.700000f, 0.350000f, 0);
    }
    if (Pose == 1) // crouching
    {
        aabb.Min = Vector4(-0.350000f, 0.000000f, -0.350000f, 0);
        aabb.Max = Vector4(0.350000f, 1.150000f, 0.350000f, 0);
    }
    if (Pose == 2) // prone
    {
        aabb.Min = Vector4(-0.350000f, 0.000000f, -0.350000f, 0);
        aabb.Max = Vector4(0.350000f, 0.400000f, 0.350000f, 0);
    }

    return aabb;
}

void DrawBox(AxisAlignedBox aabb, Vector3 m_Position, float Yaw, ImColor color)
{
    float cosY = (float)cos(Yaw);
    float sinY = (float)sin(Yaw);

    Vector3 fld = Vector3(aabb.Min.z * cosY - aabb.Min.x * sinY, aabb.Min.y, aabb.Min.x * cosY + aabb.Min.z * sinY) + m_Position; // 0
    Vector3 brt = Vector3(aabb.Min.z * cosY - aabb.Max.x * sinY, aabb.Min.y, aabb.Max.x * cosY + aabb.Min.z * sinY) + m_Position; // 1
    Vector3 bld = Vector3(aabb.Max.z * cosY - aabb.Max.x * sinY, aabb.Min.y, aabb.Max.x * cosY + aabb.Max.z * sinY) + m_Position; // 2
    Vector3 frt = Vector3(aabb.Max.z * cosY - aabb.Min.x * sinY, aabb.Min.y, aabb.Min.x * cosY + aabb.Max.z * sinY) + m_Position; // 3
    Vector3 frd = Vector3(aabb.Max.z * cosY - aabb.Min.x * sinY, aabb.Max.y, aabb.Min.x * cosY + aabb.Max.z * sinY) + m_Position; // 4
    Vector3 brb = Vector3(aabb.Min.z * cosY - aabb.Min.x * sinY, aabb.Max.y, aabb.Min.x * cosY + aabb.Min.z * sinY) + m_Position; // 5
    Vector3 blt = Vector3(aabb.Min.z * cosY - aabb.Max.x * sinY, aabb.Max.y, aabb.Max.x * cosY + aabb.Min.z * sinY) + m_Position; // 6
    Vector3 flt = Vector3(aabb.Max.z * cosY - aabb.Max.x * sinY, aabb.Max.y, aabb.Max.x * cosY + aabb.Max.z * sinY) + m_Position; // 7

    if (!W2S(fld, fld) || !W2S(brt, brt)
        || !W2S(bld, bld) || !W2S(frt, frt)
        || !W2S(frd, frd) || !W2S(brb, brb)
        || !W2S(blt, blt) || !W2S(flt, flt))
        return;

    BoxHeight = fld.y - brb.y;

    if (m_box && m_boxstyle == 1)
    {
        v.Line(ImVec2(fld.x, fld.y), ImVec2(brt.x, brt.y), color, 1.f);
        v.Line(ImVec2(brb.x, brb.y), ImVec2(blt.x, blt.y), color, 1.f);
        v.Line(ImVec2(fld.x, fld.y), ImVec2(brb.x, brb.y), color, 1.f);
        v.Line(ImVec2(brt.x, brt.y), ImVec2(blt.x, blt.y), color, 1.f);
        v.Line(ImVec2(frt.x, frt.y), ImVec2(bld.x, bld.y), color, 1.f);
        v.Line(ImVec2(frd.x, frd.y), ImVec2(flt.x, flt.y), color, 1.f);
        v.Line(ImVec2(frt.x, frt.y), ImVec2(frd.x, frd.y), color, 1.f);
        v.Line(ImVec2(bld.x, bld.y), ImVec2(flt.x, flt.y), color, 1.f);
        v.Line(ImVec2(frt.x, frt.y), ImVec2(fld.x, fld.y), color, 1.f);
        v.Line(ImVec2(frd.x, frd.y), ImVec2(brb.x, brb.y), color, 1.f);
        v.Line(ImVec2(brt.x, brt.y), ImVec2(bld.x, bld.y), color, 1.f);
        v.Line(ImVec2(blt.x, blt.y), ImVec2(flt.x, flt.y), color, 1.f);
    }
}

float GetDistance(Vector3 value1, Vector3 value2)
{
    float num = value1.x - value2.x;
    float num2 = value1.y - value2.y;
    float num3 = value1.z - value2.z;

    return sqrt(num * num + num2 * num2 + num3 * num3);
}