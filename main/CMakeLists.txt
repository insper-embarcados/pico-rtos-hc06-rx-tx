add_executable(pico_emb
	hc06.c
        main.c
)

set_target_properties(pico_emb PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_include_directories(pico_emb PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(pico_emb pico_stdlib hardware_uart hardware_gpio freertos)

# Disable Pico SDK 2.x MSPLIM stack guard — conflicts with FreeRTOS context switching on CM33
target_compile_definitions(pico_emb PRIVATE PICO_RUNTIME_NO_INIT_PER_CORE_INSTALL_STACK_GUARD=1)

pico_enable_stdio_usb(pico_emb 0)
pico_enable_stdio_uart(pico_emb 1)
pico_add_extra_outputs(pico_emb)
