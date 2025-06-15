#include "Window.h"

#include <iostream>

namespace Aleng
{
    static bool s_glfwInitialized = false;

    Window::Window(const std::string &title, int width, int height)
        : m_Title(title), m_Width(width), m_Height(height)
    {
        if (!s_glfwInitialized)
        {
            if (!glfwInit())
            {
                std::cout << "Fatal Error: glfw cannot be initialized.\n";
                std::exit(1);
            }
            s_glfwInitialized = true;
        }

        m_Handler = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

        glfwMakeContextCurrent(m_Handler);
    }

    void Window::SetActive(bool active)
    {
        if (active)
            glfwShowWindow(m_Handler);
        else
            glfwHideWindow(m_Handler);
    }

    void Window::Update()
    {
        glfwSwapInterval(1);

        while (!glfwWindowShouldClose(m_Handler))
        {
            glfwPollEvents();
            glfwSwapBuffers(m_Handler);
        }
    }

    void Window::Terminate()
    {
        glfwDestroyWindow(m_Handler);
    }
}