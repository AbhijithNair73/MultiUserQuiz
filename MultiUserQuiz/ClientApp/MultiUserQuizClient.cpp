// main.cpp - Entry point for the client application
#include "QuizClientUI.hpp"
#include <iostream>
#include <signal.h>

QuizClientUI * g_ui = nullptr;

void SignalHandler (int signal)
{
    std::cout << "\nReceived signal " << signal << ". Shutting down..." << std::endl;
    if (g_ui) {
        g_ui->Stop ();
    }
    exit (0);
}

int main ()
{
// Set up signal handling for graceful shutdown
    signal (SIGINT, SignalHandler);
    signal (SIGTERM, SignalHandler);

    try {
        QuizClientUI ui;
        g_ui = &ui;

        ui.Run ();

    } catch (const std::exception & e) {
        std::cerr << "Application error: " << e.what () << std::endl;
        return 1;
    }

    std::cout << "Quiz client terminated." << std::endl;
    return 0;
}
