#include <unittest.h>
#include <lib/env.h>
#include <stdlib.h>
#include <string.h>

extern int do_printenv(int argc, void *);
extern void env_reset(void);

/* Before each test case is called, empty out the environment */
void test_env_suite_setup(void)
{
	env_reset();
}

void test_env(uintptr_t context)
{
	const uint8_t source[] =
		"numvar=1234567890\0"
		"truevar=true\0"
		"falsevar=false\0"
		"zerovar=0\0"
		"strvar=hi mom 123=abc-_*\0"
		"ethaddr=12:34:56:78:ab:cd\0"
		"ipaddr=32.1.0.100\0"
		"blacklisted=nocando\0"
		"com.apple.System.hideme=hidden";
	uint8_t ethaddr[6];
	uint32_t ipaddr;

	TEST_ASSERT_EQ(env_unserialize(source, sizeof(source)), 0);

	// Test getting variables as strings
	TEST_ASSERT_STR_EQ(env_get("numvar"), "1234567890");
	TEST_ASSERT_STR_EQ(env_get("truevar"), "true");
	TEST_ASSERT_STR_EQ(env_get("falsevar"), "false");
	TEST_ASSERT_STR_EQ(env_get("zerovar"), "0");
	TEST_ASSERT_STR_EQ(env_get("strvar"), "hi mom 123=abc-_*");
	TEST_ASSERT_STR_EQ(env_get("ethaddr"), "12:34:56:78:ab:cd");
	TEST_ASSERT_STR_EQ(env_get("ipaddr"), "32.1.0.100");
	TEST_ASSERT_STR_EQ(env_get("com.apple.System.hideme"), "hidden");

	// Test getting variables as booleans
	TEST_ASSERT_EQ(env_get_bool("truevar", false), true);
	TEST_ASSERT_EQ(env_get_bool("truevar", true), true);
	TEST_ASSERT_EQ(env_get_bool("falsevar", false), false);
	TEST_ASSERT_EQ(env_get_bool("falsevar", true), false);
	TEST_ASSERT_EQ(env_get_bool("zerovar", false), false);
	TEST_ASSERT_EQ(env_get_bool("zerovar", true), false);
	TEST_ASSERT_EQ(env_get_bool("numvar", false), true);
	TEST_ASSERT_EQ(env_get_bool("numvar", true), true);
	TEST_ASSERT_EQ(env_get_bool("strvar", false), false);
	TEST_ASSERT_EQ(env_get_bool("strvar", true), false);
	// Test default value for env_get_bool
	TEST_ASSERT_EQ(env_get_bool("notavar", true), true);
	TEST_ASSERT_EQ(env_get_bool("notavar", false), false);

	// Test getting variables as numbers
	TEST_ASSERT_EQ(env_get_uint("numvar", 0), 1234567890ULL);
	// Test default value for env_get_uint
	TEST_ASSERT_EQ(env_get_uint("notavar", 123456), 123456);

	// Test getting variables as MAC addresses
	TEST_ASSERT_EQ(env_get_ethaddr("ethaddr", ethaddr), 0);
	TEST_ASSERT_MEM_EQ(ethaddr, TEST_ARRAY(uint8_t, 0x12, 0x34, 0x56, 0x78, 0xab, 0xcd), sizeof(ethaddr));
	// Test rejecting malformed MAC addresses
	TEST_ASSERT_NEQ(env_get_ethaddr("strvar", ethaddr), 0);
	TEST_ASSERT_NEQ(env_get_ethaddr("numvar", ethaddr), 0);
	TEST_ASSERT_NEQ(env_get_ethaddr("notavar", ethaddr), 0);

	// Test getting variables as IP addresses
	TEST_ASSERT_EQ(env_get_ipaddr("ipaddr", &ipaddr), 0);
	TEST_ASSERT_EQ(ipaddr, 0x64000120);

	// Test getting a blacklisted variable fails
	TEST_ASSERT_NULL(env_get("blacklisted"));

	// Test setting a non-persistent variable
	TEST_ASSERT_EQ(env_set("temp", "don't save me", 0), 0);
	TEST_ASSERT_STR_EQ(env_get("temp"), "don't save me");

	// Test setting a persistent variable
	TEST_ASSERT_EQ(env_set("persist", "save this var", ENV_PERSISTENT), 0);
	TEST_ASSERT_STR_EQ(env_get("persist"), "save this var");

	// Test env_set_uint
	TEST_ASSERT_EQ(env_set_uint("uint", 0xabcd1234, 0), 0);
	TEST_ASSERT_EQ(env_get_uint("uint", 0), 0xabcd1234);

	// Test overwriting a persistent variable
	TEST_ASSERT_EQ(env_set("overwrite", "will not be saved", ENV_PERSISTENT), 0);
	TEST_ASSERT_STR_EQ(env_get("overwrite"), "will not be saved");
	TEST_ASSERT_EQ(env_set("overwrite", "save this value", ENV_PERSISTENT), 0);
	TEST_ASSERT_STR_EQ(env_get("overwrite"), "save this value");

	// Test unsetting a value
	TEST_ASSERT_EQ(env_set("unset", "about to unset", ENV_PERSISTENT), 0);
	TEST_ASSERT_STR_EQ(env_get("unset"), "about to unset");
	TEST_ASSERT_EQ(env_unset("unset"), 1);
	TEST_ASSERT_NULL(env_get("unset"));
	// unsetting a non-existant variable should fail
	TEST_ASSERT_EQ(env_unset("unset"), 0);

	// Test serializing - everything from the source should be serialized,
	// plus persistent variables set with env_set
	const uint8_t expected[] =
		"numvar=1234567890\0"
		"truevar=true\0"
		"falsevar=false\0"
		"zerovar=0\0"
		"strvar=hi mom 123=abc-_*\0"
		"ethaddr=12:34:56:78:ab:cd\0"
		"ipaddr=32.1.0.100\0"
		"blacklisted=nocando\0"
		"com.apple.System.hideme=hidden\0"
		"persist=save this var\0"
		"overwrite=save this value";
	
	uint8_t *serialized = malloc(sizeof(expected) * 2);

	TEST_ASSERT_EQ(env_serialize(serialized, sizeof(expected) * 2), sizeof(expected));
	TEST_ASSERT_MEM_EQ(serialized, expected, sizeof(expected));

	memset(serialized, 0, sizeof(expected));
	TEST_ASSERT_EQ(env_serialize(serialized, sizeof(expected) + 1), sizeof(expected));
	TEST_ASSERT_MEM_EQ(serialized, expected, sizeof(expected));

	// Too small buffer should fail serialization
	TEST_ASSERT_EQ(env_serialize(serialized, sizeof(expected) - 1), 0);
}

void test_env_max_len_var(uintptr_t context)
{
	int result;
	size_t serialized_len;
	const char *long_var =
		"0001020304050607"
		"08090a0b0c0d0e0f"
		"1011121314151617"
		"18191a1b1c1d1e1f"
		"2021222324252627"
		"28292a2b2c2d2e2f"
		"3031323334353637"
		"38393a3b3c3d3e3f"
		"4041424344454647"
		"48494a4b4c4d4e4f"
		"5051525354555657"
		"58595a5b5c5d5e5f"
		"6061626364656667"
		"68696a6b6c6d6e6f"
		"7071727374757677"
		"78797a7b7c7d7e_";

	const uint8_t expected[] =
		"testvar="
		"0001020304050607"
		"08090a0b0c0d0e0f"
		"1011121314151617"
		"18191a1b1c1d1e1f"
		"2021222324252627"
		"28292a2b2c2d2e2f"
		"3031323334353637"
		"38393a3b3c3d3e3f"
		"4041424344454647"
		"48494a4b4c4d4e4f"
		"5051525354555657"
		"58595a5b5c5d5e5f"
		"6061626364656667"
		"68696a6b6c6d6e6f"
		"7071727374757677"
		"78797a7b7c7d7e_"
		"\0testvar2=abc";

	// Setting the variable should succeed
	result = env_set("testvar", long_var, ENV_PERSISTENT);
	TEST_ASSERT_EQ(result, 0);

	// Add a dummy variable after testvar for serialization test
	result = env_set("testvar2", "abc", ENV_PERSISTENT);
	TEST_ASSERT_EQ(result, 0);

	// The variable should read back correctly
	TEST_ASSERT_STR_EQ(env_get("testvar"), long_var);

	// The variable should serialize correctly
	uint8_t *serialized = malloc(sizeof(expected) * 2);
	serialized_len = env_serialize(serialized, sizeof(expected) * 2);
	TEST_ASSERT_EQ(serialized_len, sizeof(expected));
	TEST_ASSERT_MEM_EQ(serialized, expected, sizeof(expected));

	// Clear out the environment to prepare for deserialization
	env_reset();

	// Make sure the environment really was cleared
	TEST_ASSERT_NULL(env_get("testvar"));
	TEST_ASSERT_NULL(env_get("testvar2"));

	// Now load the environment back in from the serialized buffer
	result = env_unserialize(serialized, serialized_len);
	TEST_ASSERT_EQ(result, 0);

	// And make sure everything came through OK
	TEST_ASSERT_STR_EQ(env_get("testvar"), long_var);
	TEST_ASSERT_STR_EQ(env_get("testvar2"), "abc");
}

bool env_blacklist(const char *name, bool write)
{
	if (strcmp(name, "set-blacklisted") == 0)
		return true;
	return false;
}

bool env_blacklist_nvram(const char *name)
{
	if (strcmp(name, "blacklisted") == 0)
		return true;
	return false;
}

static struct test_suite env_test_suite = {
	.name = "env",
	.description = "tests the environment module",
	.setup_function = test_env_suite_setup,
	.test_cases = {
		{ "env", test_env, 0 },
		{ "env_max_len_var", test_env_max_len_var, 0 },
		TEST_CASE_LAST
	}
};

TEST_SUITE(env_test_suite);
