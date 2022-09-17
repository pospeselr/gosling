using namespace std;
using namespace gosling;

// platform specific wrappers for tcp stream stuffs
#if defined(GOSLING_PLATFORM_WINDOWS)
typedef SOCKET tcp_stream_t ;
#elif (defined(GOSLING_PLATFORM_MACOS) || defined(GOSLING_PLATFORM_LINUX))
typedef int tcp_stream_t;
#endif

// simple bson document: { msg : "hello world" }
constexpr static uint8_t challenge_bson[] = {
    // document length 26 == 0x0000001a
    0x1a,0x00,0x00,0x00,
    // string msg
    0x02,'m','s','g',0x00,
    // strlen("hello world\x00") 12 = 0x0000000c
    0x0c,0x00,0x00,0x00,
    // "hello world"
    'h','e','l','l','o',' ','w','o','r','l','d',0x00,
    0x00
};

// empty document
constexpr static uint8_t  challenge_response_bson[] = {
    0x05,0x00,0x00,0x00,
    0x00
};

static void create_client_handshake(unique_ptr<gosling_context>& ctx) {
    const auto init_callback = []() -> size_t {
        auto handshake_handle = 0;
        cout << "--- started_callback: { handshake_handle: " << handshake_handle << " }" <<endl;
        return handshake_handle;
    };
    REQUIRE_NOTHROW(::gosling_context_set_identity_client_handshake_init_callback(
        ctx.get(),
        init_callback,
        throw_on_error()));

    const auto challenge_response_size_callback = [](
        const size_t* handshake_handle,
        const char* endpoint_name,
        size_t endpoint_name_length) -> size_t {
        REQUIRE(handshake_handle != nullptr);
        REQUIRE(*handshake_handle == 0);
        cout << "--- challenge_response_size_callback: { handshake_handle: " << *handshake_handle << " }" <<endl;
        return sizeof(challenge_response_bson);
    };

    REQUIRE_NOTHROW(::gosling_context_set_identity_client_challenge_response_size_callback(
        ctx.get(),
        challenge_response_size_callback,
        throw_on_error()));

    const auto build_challenge_response_callback = [](
        const size_t* handshake_handle,
        const char *endpoint_name,
        size_t endpoint_name_length,
        const uint8_t *challenge_buffer,
        size_t challenge_buffer_size,
        uint8_t *out_challenge_response_buffer,
        size_t challenge_response_buffer_size) -> void {
        REQUIRE(handshake_handle != nullptr);
        REQUIRE(*handshake_handle == 0);
        cout << "--- build_challenge_response_callback: { handshake_handle: " << *handshake_handle << " }" <<endl;

        REQUIRE(string(endpoint_name, endpoint_name_length) == "default");
        REQUIRE(challenge_buffer_size == sizeof(challenge_bson));
        REQUIRE(std::equal(challenge_buffer, challenge_buffer + challenge_buffer_size, challenge_bson));
        REQUIRE(challenge_response_buffer_size == sizeof(challenge_response_bson));

        std::copy(challenge_response_bson, challenge_response_bson + sizeof(challenge_response_bson), out_challenge_response_buffer);
    };

    REQUIRE_NOTHROW(::gosling_context_set_identity_client_build_challenge_response_callback(
        ctx.get(),
        build_challenge_response_callback,
        throw_on_error()));

}

static void create_server_handshake(unique_ptr<gosling_context>& ctx) {
    const auto init_callback = []() -> size_t {
        auto handshake_handle = 0;
        cout << "--- started_callback: { handshake_handle: " << handshake_handle << " }" <<endl;
        return handshake_handle;
    };

    REQUIRE_NOTHROW(::gosling_context_set_identity_server_handshake_init_callback(
        ctx.get(),
        init_callback,
        throw_on_error()));

    const auto endpoint_supported_callback = [](
        const size_t* handshake_handle,
        const char* endpoint_name,
        size_t endpoint_name_length) -> bool {
        REQUIRE(handshake_handle != nullptr);
        REQUIRE(*handshake_handle == 0);
        cout << "--- endpoint_supported_callback: { handshake_handle: " << *handshake_handle << " }" <<endl;


        if (string(endpoint_name, endpoint_name_length) == "default") {
            return true;
        }

        return false;
    };

    REQUIRE_NOTHROW(::gosling_context_set_identity_server_endpoint_supported_callback(
        ctx.get(),
        endpoint_supported_callback,
        throw_on_error()));

    const auto challenge_size_callback = [](
        const size_t* handshake_handle,
        const char* endpoint_name,
        size_t endpoint_name_length) -> size_t {
        REQUIRE(handshake_handle != nullptr);
        REQUIRE(*handshake_handle == 0);
        cout << "--- challenge_size_callback: { handshake_handle: " << *handshake_handle << " }" <<endl;
        return sizeof(challenge_bson);
    };

    REQUIRE_NOTHROW(::gosling_context_set_identity_server_challenge_size_callack(
        ctx.get(),
        challenge_size_callback,
        throw_on_error()));

    const auto build_challenge_callback = [](
        const size_t* handshake_handle,
        const char* endpoint_name,
        size_t endpoint_name_length,
        uint8_t* out_challenge_buffer,
        size_t challenge_buffer_size) -> void {
        REQUIRE(handshake_handle != nullptr);
        REQUIRE(*handshake_handle == 0);
        cout << "--- build_challenge_callback: { handshake_handle: " << *handshake_handle << " }" <<endl;

        REQUIRE(string(endpoint_name, endpoint_name_length) == "default");
        REQUIRE(out_challenge_buffer != nullptr);
        REQUIRE(challenge_buffer_size == sizeof(challenge_bson));

        std::copy(challenge_bson, challenge_bson + sizeof(challenge_bson), out_challenge_buffer);
    };

    REQUIRE_NOTHROW(::gosling_context_set_identity_server_build_challenge_callback(
        ctx.get(),
        build_challenge_callback,
        throw_on_error()));

    const auto verify_challenge_response_callback = [](
        const size_t* handshake_handle,
        const char* endpoint_name,
        size_t endpoint_name_length,
        const uint8_t* challenge_buffer,
        size_t challenge_buffer_size,
        const uint8_t* challenge_response_buffer,
        size_t challenge_response_buffer_size) -> gosling_challenge_response_result_t {
        REQUIRE(handshake_handle != nullptr);
        REQUIRE(*handshake_handle == 0);
        cout << "--- verify_challenge_response_callback: { handshake_handle: " << *handshake_handle << " }" <<endl;

        REQUIRE(string(endpoint_name, endpoint_name_length) == "default");
        REQUIRE(challenge_buffer != nullptr);
        REQUIRE(challenge_buffer_size == sizeof(challenge_bson));
        REQUIRE(std::equal(challenge_buffer, challenge_buffer + challenge_buffer_size, challenge_bson));
        REQUIRE(challenge_response_buffer != nullptr);

        if (challenge_response_buffer_size != sizeof(challenge_response_bson)) {
            return gosling_challenge_response_result_invalid;
        }

        if (!std::equal(challenge_response_buffer, challenge_response_buffer + challenge_response_buffer_size, challenge_response_bson)) {
            return gosling_challenge_response_result_invalid;
        }

        return gosling_challenge_response_result_valid;
    };

    REQUIRE_NOTHROW(::gosling_context_set_identity_server_verify_challenge_response_callback(
        ctx.get(),
        verify_challenge_response_callback,
        throw_on_error()));

    const auto poll_challenge_response_result_callback = [](
        const size_t* handshake_handle) -> gosling_challenge_response_result_t {
        REQUIRE(handshake_handle != nullptr);
        REQUIRE(*handshake_handle == 0);
        cout << "--- poll_challenge_response_result_callback: { handshake_handle: " << *handshake_handle << " }" <<endl;
        return gosling_challenge_response_result_pending;
    };

    REQUIRE_NOTHROW(::gosling_context_set_identity_server_poll_challenge_response_result_callback(
        ctx.get(),
        poll_challenge_response_result_callback,
        throw_on_error()));

}

// gosling demo
TEST_CASE("gosling_cpp_demo") {

    // generate private keys
    unique_ptr<gosling_ed25519_private_key> alice_private_key;
    REQUIRE_NOTHROW(::gosling_ed25519_private_key_generate(out(alice_private_key), throw_on_error()));

    cout << "alice key: " << alice_private_key.get() << endl;

    unique_ptr<gosling_ed25519_private_key> pat_private_key;
    REQUIRE_NOTHROW(::gosling_ed25519_private_key_generate(out(pat_private_key), throw_on_error()));

    cout << "pat key: " << pat_private_key.get() << endl;

    // calculate service ids
    unique_ptr<gosling_v3_onion_service_id> alice_identity;
    REQUIRE_NOTHROW(::gosling_v3_onion_service_id_from_ed25519_private_key(out(alice_identity), alice_private_key.get(), throw_on_error()));

    cout << "alice service id: " << alice_identity.get() << endl;

    unique_ptr<gosling_v3_onion_service_id> pat_identity;
    REQUIRE_NOTHROW(::gosling_v3_onion_service_id_from_ed25519_private_key(out(pat_identity), pat_private_key.get(), throw_on_error()));

    cout << "pat service id: " << pat_identity.get() << endl;

    // init contexts
    unique_ptr<gosling_context> alice_context;
    string_view alice_working_dir = "/tmp/gosling_context_test_alice";
    REQUIRE_NOTHROW(::gosling_context_init(
        out(alice_context), // out_context
        alice_working_dir.data(), // tor working dirctory
        alice_working_dir.size(), // tor working directory len
        420,  // identity port
        420,  // endpoint port
        alice_private_key.get(), // identity private key
        nullptr, // blocked clients,
        0, // blocked clients count
        throw_on_error()));

    create_client_handshake(alice_context); // client callbacks
    create_server_handshake(alice_context); // server callbacks

    unique_ptr<gosling_context> pat_context;
    string_view pat_working_dir = "/tmp/gosling_context_test_pat";
    REQUIRE_NOTHROW(::gosling_context_init(
        out(pat_context), // out_context
        pat_working_dir.data(), // tor working dirctory
        pat_working_dir.size(), // tor working directory len
        420,  // identity port
        420,  // endpoint port
        alice_private_key.get(), // identity private key
        nullptr, // blocked clients,
        0, // blocked clients count
        throw_on_error()));

    create_client_handshake(pat_context); // client callbacks
    create_server_handshake(pat_context); // server callbacks

    // bootstrap alice
    static bool alice_bootstrap_complete = false;

    REQUIRE_NOTHROW(::gosling_context_set_tor_bootstrap_completed_callback(
        alice_context.get(),
        [](gosling_context* context) -> void {
            alice_bootstrap_complete = true;
            cout << "--- alice bootstrapped" << endl;
        },
        throw_on_error()));
    REQUIRE_NOTHROW(::gosling_context_bootstrap_tor(alice_context.get(), throw_on_error()));

    while(!alice_bootstrap_complete) {
        REQUIRE_NOTHROW(::gosling_context_poll_events(alice_context.get(), throw_on_error()));
    }

    // init alice's identity server
    static bool alice_identity_server_ready = false;
    REQUIRE_NOTHROW(::gosling_context_set_identity_server_published_callback(alice_context.get(),
        [](gosling_context* context) -> void {
            alice_identity_server_ready = true;
            cout << "--- alice identity server published" << endl;
        },
        throw_on_error()));
    REQUIRE_NOTHROW(::gosling_context_start_identity_server(alice_context.get(), throw_on_error()));

    while(!alice_identity_server_ready) {
        REQUIRE_NOTHROW(::gosling_context_poll_events(alice_context.get(), throw_on_error()));
    }

    // bootstrap pat
    static bool pat_bootstrap_complete = false;
    REQUIRE_NOTHROW(::gosling_context_set_tor_bootstrap_completed_callback(
        pat_context.get(),
        [](gosling_context* context) -> void {
            pat_bootstrap_complete = true;
            cout << "--- pat bootstrapped" << endl;
        },
        throw_on_error()));
    REQUIRE_NOTHROW(::gosling_context_bootstrap_tor(pat_context.get(), throw_on_error()));

    while(!pat_bootstrap_complete) {
        REQUIRE_NOTHROW(::gosling_context_poll_events(alice_context.get(), throw_on_error()));
        REQUIRE_NOTHROW(::gosling_context_poll_events(pat_context.get(), throw_on_error()));
    }

    // pat requests an endpoint from alice
    static bool pat_endpoint_request_complete = false;
    static unique_ptr<gosling_v3_onion_service_id> alice_endpoint_service_id;
    static unique_ptr<gosling_x25519_private_key> pat_onion_auth_private_key;
    std::string endpointName = "default";
    REQUIRE_NOTHROW(::gosling_context_set_endpoint_client_request_completed_callback(pat_context.get(),
        [](
            gosling_context* context,
            const gosling_v3_onion_service_id* identity_service_id,
            const gosling_v3_onion_service_id* endpoint_service_id,
            const char* endpoint_name,
            size_t endpoint_name_length,
            const gosling_x25519_private_key* client_auth_private_key) -> void {

            REQUIRE(string(endpoint_name, endpoint_name_length) == "default");

            REQUIRE_NOTHROW(::gosling_v3_onion_service_id_clone(out(alice_endpoint_service_id), endpoint_service_id, throw_on_error()));
            REQUIRE_NOTHROW(::gosling_x25519_private_key_clone(out(pat_onion_auth_private_key), client_auth_private_key, throw_on_error()));

            pat_endpoint_request_complete = true;
            cout << "--- pat endpoint request completed" << endl;
        },
        throw_on_error()));
    static bool alice_endpoint_request_complete = false;
    static unique_ptr<gosling_ed25519_private_key> alice_endpoint_private_key;
    static unique_ptr<gosling_v3_onion_service_id> pat_identity_service_id;
    static unique_ptr<gosling_x25519_public_key> pat_onion_auth_public_key;
    REQUIRE_NOTHROW(::gosling_context_set_endpoint_server_request_completed_callback(alice_context.get(),
        [](
            gosling_context* context,
            const gosling_ed25519_private_key* endpoint_private_key,
            const char* endpoint_name,
            size_t endpoint_name_length,
            const gosling_v3_onion_service_id* client_service_id,
            const gosling_x25519_public_key* client_auth_public_key) -> void {

            REQUIRE(string(endpoint_name, endpoint_name_length) == "default");

            REQUIRE_NOTHROW(::gosling_ed25519_private_key_clone(out(alice_endpoint_private_key), endpoint_private_key, throw_on_error()));
            REQUIRE_NOTHROW(::gosling_v3_onion_service_id_clone(out(pat_identity_service_id), client_service_id, throw_on_error()));
            REQUIRE_NOTHROW(::gosling_x25519_public_key_clone(out(pat_onion_auth_public_key), client_auth_public_key, throw_on_error()));

            alice_endpoint_request_complete = true;
            cout << "--- alice endpoint request completed" << endl;
        },
        throw_on_error()));
    REQUIRE_NOTHROW(::gosling_context_request_remote_endpoint(pat_context.get(), alice_identity.get(), endpointName.data(), endpointName.size(), throw_on_error()));

    while(!alice_endpoint_request_complete) {
        REQUIRE_NOTHROW(::gosling_context_poll_events(alice_context.get(), throw_on_error()));
        REQUIRE_NOTHROW(::gosling_context_poll_events(pat_context.get(), throw_on_error()));
    }

    // alice stand's up endpoint server
    static bool alice_endpoint_published = false;
    REQUIRE_NOTHROW(::gosling_context_set_endpoint_server_published_callback(alice_context.get(),
        [](
            gosling_context* context,
            const gosling_v3_onion_service_id* endpoint_service_id,
            const char* endpoint_name,
            size_t endpoint_name_length) -> void {

            REQUIRE(string(endpoint_name, endpoint_name_length) == "default");
            alice_endpoint_published = true;
            cout << "--- alice endpoint published" << endl;
        },
        throw_on_error()));
    REQUIRE_NOTHROW(::gosling_context_start_endpoint_server(
        alice_context.get(),
        alice_endpoint_private_key.get(),
        endpointName.data(),
        endpointName.size(),
        pat_identity_service_id.get(),
        pat_onion_auth_public_key.get(),
        throw_on_error()));

    while(!alice_endpoint_published || !pat_endpoint_request_complete) {
        REQUIRE_NOTHROW(::gosling_context_poll_events(alice_context.get(), throw_on_error()));
        REQUIRE_NOTHROW(::gosling_context_poll_events(pat_context.get(), throw_on_error()));
    }

    // pat connects to alice's endpoint
    static bool pat_channel_request_complete = false;
    static bool alice_channel_request_complete = false;
    static tcp_stream_t pat_stream = tcp_stream_t();
    static tcp_stream_t alice_stream = tcp_stream_t();

    static boost::asio::io_service io_service;
    static boost::asio::ip::tcp::socket pat_socket(io_service);
    static boost::asio::ip::tcp::socket alice_socket(io_service);

    string channelName("funky");
    REQUIRE_NOTHROW(::gosling_context_set_endpoint_client_channel_request_completed_callback(pat_context.get(),
        [](
            gosling_context* context,
            const gosling_v3_onion_service_id* endpoint_service_id,
            const char* channel_name,
            size_t channel_name_length,
            tcp_stream_t stream) -> void {

            REQUIRE(string(channel_name, channel_name_length) == "funky");

            cout << "--- pat channel request complete" << endl;
            pat_channel_request_complete = true;
            pat_socket.assign(boost::asio::ip::tcp::v4(), stream);

        },
        throw_on_error()));
    REQUIRE_NOTHROW(::gosling_context_set_endpoint_server_channel_request_completed_callback(alice_context.get(),
        [](
            gosling_context* context,
            const gosling_v3_onion_service_id* endpoint_service_id,
            const gosling_v3_onion_service_id* client_service_id,
            const char* channel_name,
            size_t channel_name_length,
            tcp_stream_t stream) -> void {

            REQUIRE(string(channel_name, channel_name_length) == "funky");
            cout << "--- alice channel request complete" << endl;
            alice_channel_request_complete = true;
            alice_socket.assign(boost::asio::ip::tcp::v4(), stream);
        },
        throw_on_error()));

    // pat opens chanel to alice's endpoint
    REQUIRE_NOTHROW(::gosling_context_open_endpoint_channel(
        pat_context.get(),
        alice_endpoint_service_id.get(),
        pat_onion_auth_private_key.get(),
        channelName.data(),
        channelName.size(),
        throw_on_error()));

    // wait for both channels to be open
    while(!pat_channel_request_complete || !alice_channel_request_complete) {
        REQUIRE_NOTHROW(::gosling_context_poll_events(alice_context.get(), throw_on_error()));
        REQUIRE_NOTHROW(::gosling_context_poll_events(pat_context.get(), throw_on_error()));
    }

    // pat sends Hello Alice to alice
    std::string pat_message = "Hello Alice!\n";
    std::string alice_read_buffer;

    cout << "--- pat writes message" << endl;

    boost::asio::write(pat_socket, boost::asio::buffer(pat_message.data(), pat_message.size()));

    cout << "--- alice waits for message" << endl;

    // alice reads
    boost::asio::read_until(alice_socket, boost::asio::dynamic_buffer(alice_read_buffer), '\n');
    REQUIRE(pat_message == alice_read_buffer);

    // remove the trailing new-line byte
    alice_read_buffer.pop_back();

    cout << "--- alice received '" << alice_read_buffer << "'" << endl;
}
