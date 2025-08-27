include(GoogleTest)

add_executable(drop-box-tests test_main.cpp)

target_link_libraries(drop-box-tests
    PRIVATE
        GTest::gtest_main
        drop-box-core
)

gtest_discover_tests(drop-box-tests)

add_executable(encryption_manager_tests EncryptionManagerTest.cpp)

target_link_libraries(encryption_manager_tests
    PRIVATE
        GTest::gtest_main
        drop-box-core
)

gtest_discover_tests(encryption_manager_tests)