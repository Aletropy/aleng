#pragma once

#include <string>
#include <GLFW/glfw3.h>

namespace Aleng
{
    class Window
    {
    public:
        Window(const std::string &title, int width = 800, int height = 600);

        void SetActive(bool active);
        void Update();
        void Terminate();

        GLFWwindow *GetHandler() { return m_Handler; }

    private:
        GLFWwindow *m_Handler;
        std::string m_Title;
        int m_Width, m_Height;
    };
}