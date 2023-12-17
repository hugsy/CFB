#define CATCH_CONFIG_MAIN

#include <catch2/catch_test_macros.hpp>
#define NS "CFB::Utils"

#include "Utils.hpp"

TEST_CASE("Strings", NS)
{
    SECTION("ToString(WideString)")
    {
        CHECK(CFB::Utils::ToString(L"FOOBAR") == "FOOBAR");
        CHECK(CFB::Utils::ToString(L"FOOBAR").size() == 6);
    }

    SECTION("ToWideString(String)")
    {
        CHECK(CFB::Utils::ToWideString("FOOBAR") == L"FOOBAR");
        CHECK(CFB::Utils::ToWideString("FOOBAR").size() == 6);
    }

    SECTION("IRP")
    {
        for ( int i = 0; i < 0x1c; i++ )
        {
            CHECK(std::string(CFB::Utils::IrpMajorToString(i)).starts_with("IRP_"));
        }

        CHECK(std::string(CFB::Utils::IrpMajorToString(0x4242)) == "UnknownIrpType");
    }
}

TEST_CASE("Memory", NS)
{
    CHECK(CFB::Utils::Memory::IsAligned(0x41414141, 0x10) == false);
    CHECK(CFB::Utils::Memory::IsAligned(0x41414140, 0x10) == true);

    CHECK(CFB::Utils::Memory::AlignValue(0x41414100, 0x100) == 0x41414100);
    for ( int i = 1; i < 0x100; i++ )
    {
        CHECK(CFB::Utils::Memory::AlignValue(0x41414100 + i, 0x100) == 0x41414200);
    }
}
