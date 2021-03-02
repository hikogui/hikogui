
configure_file(demo_version.hpp.in demo_version.hpp @ONLY)

target_sources(ttauri_demo PRIVATE
    audio_preferences_controller.cpp
    audio_preferences_controller.hpp
    application_preferences.hpp
    license_preferences_controller.cpp
    license_preferences_controller.hpp
    main.cpp
    main_window_controller.cpp
    main_window_controller.hpp
    preferences_controller.cpp
    preferences_controller.hpp
    application_controller.cpp
    application_controller.hpp
)
