
juce_add_gui_app(serialist
        # VERSION ...                       # Set this if the app version is different to the project version
        # ICON_BIG ...                      # ICON_* arguments specify a path to an image file to use as an icon
        # ICON_SMALL ...
        # DOCUMENT_EXTENSIONS ...           # Specify file extensions that should be associated with this app
        # COMPANY_NAME ...                  # Specify the name of the app's author
        PRODUCT_NAME "Serialist")

target_sources(serialist
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/serialist_looper.cpp
        ${SOURCES})

target_include_directories(serialist PRIVATE
        ${INCLUDES})

target_compile_definitions(serialist
        PRIVATE
        USE_JUCE=1 # Parameter class
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:serialist,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:serialist,JUCE_VERSION>"
)

target_link_libraries(serialist
        PRIVATE
        # GuiAppData            # If we'd created a binary data target, we'd link to it here
        ${MODULE_DEPENDENCIES_JUCE}
        PUBLIC
        ${FLAGS_JUCE})
