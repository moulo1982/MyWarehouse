#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <atomic>
#include <functional>
#include <boost/asio.hpp>
#include "Base64.h"
#include <pthread.h>
using boost::asio::ip::tcp;
using namespace std;


#include <bson.h>
#include <mongoc.h>
#include "mongo/client/dbclient.h"
using namespace mongo;

#define PERF

#ifdef PERF
#define TIMES INT32_MAX
#define THREADS 200
#else
#define TIMES 1
#define THREADS 1
#endif

enum { max_length = 1024 };

std::atomic<int> ai_count(0);

void func(char **argv)
{
    boost::asio::io_service io_service;
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(tcp::v4(), argv[1], argv[2]);
    tcp::resolver::iterator iterator = resolver.resolve(query);
    tcp::socket s(io_service);

    for (int i = 1; i <= TIMES; i++)
    {
        boost::system::error_code ec;
        boost::asio::connect(s, iterator, ec);
        if (ec)
        {
            std::cout << std::this_thread::get_id() << ": " << ec.message() << std::endl;
            sleep(1);
            continue;
        }

        using namespace std;
        char b[] = "POST / HTTP/1.1\r\nHost: 120.132.85.254:8080\r\nAccept: * /*\r\nContent-Type: text/json; charset=utf-8\r\nCon";
        //char b[] = "POST / HTTP/1.1\r\nHost: 120.132.85POST / HTTP/1.1\r\nHost: 120.132.85POST / HTTP/1.1\r\nHost: 120.132.85POST / HTTP/1.1\r\nHost: 120.132.85";
        boost::asio::write(s, boost::asio::buffer(b, strlen(b)), ec);
        //std::cout << b << std::endl;

        //sleep(1);

        char b1[] = "tent-Length: 91\r\n\r\n{\n   \"ClientVersion\" : \"0.0.1\",\n   \"SceneId\" : \"SceneAndroidEnGold_1\",\n   \"UsrID\" : 8962\n}\r\n";
        boost::asio::write(s, boost::asio::buffer(b1, strlen(b1)), ec);
        //std::cout << b1 << std::endl;

        char reply[max_length] = {0};
        size_t reply_length = boost::asio::read(s,
            boost::asio::buffer(reply, max_length), ec);

        //std::cout << reply << std::endl;

        ++ai_count;


        s.shutdown(boost::asio::socket_base::shutdown_both, ec);
        s.close(ec);

        usleep(rand()%50 * 1000);
    }
}

bson_oid_t mongodb_insert()
{
    mongoc_client_t *client;
    mongoc_collection_t *collection;
    mongoc_cursor_t *cursor;
    bson_error_t error;
    bson_oid_t oid;
    bson_t *doc;

    mongoc_init();

    client = mongoc_client_new("mongodb://localhost:27017/");
    collection = mongoc_client_get_collection(client, "test", "test");

    doc = bson_new();
    bson_oid_init(&oid, NULL);
    BSON_APPEND_OID(doc, "_id", &oid);
    BSON_APPEND_UTF8(doc, "hello", "world");

    if (!mongoc_collection_insert(collection, MONGOC_INSERT_NONE, doc, NULL, &error)) {
        printf("%s\n", error.message);
    }

    bson_destroy(doc);
    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);

    return oid;
}

void mongodb_update()
{
    mongoc_collection_t *collection;
    mongoc_client_t *client;
    bson_error_t error;
    bson_oid_t oid;
    bson_t *doc = NULL;
    bson_t *update = NULL;
    bson_t *query = NULL;

    mongoc_init();

    client = mongoc_client_new("mongodb://localhost:27017/");
    collection = mongoc_client_get_collection(client, "test", "test");

    bson_oid_init(&oid, NULL);
    doc = BCON_NEW("_id", BCON_OID(&oid),
        "hello", BCON_UTF8("world!"));

    if (!mongoc_collection_insert(collection, MONGOC_INSERT_NONE, doc, NULL, &error)) {
        printf("%s\n", error.message);
        goto fail;
    }

    query = BCON_NEW("_id", BCON_OID(&oid));
    update = BCON_NEW("$set", "{",
        "hello", BCON_UTF8("Everybody!"),
        "updated", BCON_BOOL(true),
        "}");

    if (!mongoc_collection_update(collection, MONGOC_UPDATE_NONE, query, update, NULL, &error)) {
        printf("%s\n", error.message);
        goto fail;
    }

fail:
    if (doc)
        bson_destroy(doc);
    if (query)
        bson_destroy(query);
    if (update)
        bson_destroy(update);

    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);

}

void mongodb_delete()
{
    mongoc_client_t *client;
    mongoc_collection_t *collection;
    mongoc_cursor_t *cursor;
    bson_error_t error;
    bson_oid_t oid;
    bson_t *doc;

    mongoc_init();

    client = mongoc_client_new("mongodb://localhost:27017/");
    collection = mongoc_client_get_collection(client, "test", "test");

    doc = bson_new();
    bson_oid_init(&oid, NULL);
    BSON_APPEND_OID(doc, "_id", &oid);
    BSON_APPEND_UTF8(doc, "hello", "world");

    if (!mongoc_collection_insert(collection, MONGOC_INSERT_NONE, doc, NULL, &error)) {
        printf("Insert failed: %s\n", error.message);
    }

    bson_destroy(doc);

    doc = bson_new();
    BSON_APPEND_OID(doc, "_id", &oid);

    if (!mongoc_collection_remove(collection, MONGOC_REMOVE_SINGLE_REMOVE, doc, NULL, &error)) {
        printf("Delete failed: %s\n", error.message);
    }

    bson_destroy(doc);
    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);
}

void mongodb_delete(bson_oid_t oid)
{
    mongoc_client_t *client;
    mongoc_collection_t *collection;
    mongoc_cursor_t *cursor;
    bson_error_t error;
    bson_t *doc;

    mongoc_init();

    client = mongoc_client_new("mongodb://localhost:27017/");
    collection = mongoc_client_get_collection(client, "test", "test");

    doc = bson_new();
    BSON_APPEND_OID(doc, "_id", &oid);

    if (!mongoc_collection_remove(collection, MONGOC_REMOVE_SINGLE_REMOVE, doc, NULL, &error)) {
        printf("Delete failed: %s\n", error.message);
    }

    bson_destroy(doc);
    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);
}

static void
run_command(void)
{
    mongoc_client_t *client;
    mongoc_collection_t *collection;
    bson_error_t error;
    bson_t *command;
    bson_t reply;
    char *str;

    client = mongoc_client_new("mongodb://localhost:27017/");
    collection = mongoc_client_get_collection(client, "test", "test");

    command = BCON_NEW("collStats", BCON_UTF8("test"));
    if (mongoc_collection_command_simple(collection, command, NULL, &reply, &error)) {
        str = bson_as_json(&reply, NULL);
        printf("%s\n", str);
        bson_free(str);
    }
    else {
        fprintf(stderr, "Failed to run command: %s\n", error.message);
    }

    bson_destroy(command);
    bson_destroy(&reply);
    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);
}

void mongodb_find()
{
    mongoc_client_t *client;
    mongoc_collection_t *collection;
    mongoc_cursor_t *cursor;
    const bson_t *doc;
    bson_t *query;
    char *str;

    mongoc_init();

    client = mongoc_client_new("mongodb://localhost:27017/");
    collection = mongoc_client_get_collection(client, "test", "test");
    query = bson_new();
    cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);

    while (mongoc_cursor_next(cursor, &doc)) {
        str = bson_as_json(doc, NULL);
        printf("%s\n", str);
        bson_free(str);
    }

    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);
}

void mongodb_find_specific()
{
    mongoc_client_t *client;
    mongoc_collection_t *collection;
    mongoc_cursor_t *cursor;
    const bson_t *doc;
    bson_t *query;
    char *str;

    mongoc_init();

    client = mongoc_client_new("mongodb://localhost:27017/");
    collection = mongoc_client_get_collection(client, "test", "test");
    query = bson_new();
    //BSON_APPEND_UTF8(query, "hello", "world");
    BSON_APPEND_UTF8(query, "_id", "55d5504d44c1a12046052db1"); 

    cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);

    while (mongoc_cursor_next(cursor, &doc)) {
        str = bson_as_json(doc, NULL);
        printf("%s\n", str);
        bson_free(str);
    }

    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);
}

void printIfAge(mongo::DBClientConnection& c, int age) {

    std::auto_ptr<mongo::DBClientCursor> cursor =
        c.query("test.test", MONGO_QUERY("age" << age));
    while (cursor->more()) {
        mongo::BSONObj p = cursor->next();
        std::cout << p.getStringField("name") << std::endl;
    }
}

void run() {
    mongo::DBClientConnection c;
    c.connect("localhost");

    c.update("test.test",
        BSON("name" << "Joe" << "age" << 33),
        BSON("$inc" << BSON("visits" << 1))
        );

    /*
    mongo::BSONObjBuilder b;
    b.append("name", "Joe");
    b.append("age", 33);
    mongo::BSONObj p = b.obj();

    c.insert("test.test", p);
    */

    
    std::auto_ptr<mongo::DBClientCursor> cursor = c.query("test.test", mongo::BSONObj());

    while (cursor->more())
        std::cout << cursor->next().toString() << std::endl;
    

    //printIfAge(c, 33);
}

int main(int argc, char* argv[])
{
    mongo::client::initialize();
    try {
        run();
        std::cout << "connected ok" << std::endl;
    }
    catch (const mongo::DBException &e) {
        std::cout << "caught " << e.what() << std::endl;
    }


    return EXIT_SUCCESS;
    /*
    char input[] = "POST / HTTP/1.1\r\nHost: 120.132.85.254:8080\r\nAccept: * / *\r\nContent-Type: text/json; charset=utf-8\r\nContent-Length: 91\r\n\r\n{\n   \"ClientVersion\" : \"0.0.1\",\n   \"SceneId\" : \"SceneAndroidEnGold_1\",\n   \"UsrID\" : 8962\n}\r\n";
    int inputLen = sizeof(input);

    std::cout << input << std::endl;

    int encodedLen = base64_enc_len(inputLen);
    char encoded[encodedLen];

    base64_encode(encoded, input, inputLen);
    std::cout << encoded << std::endl;


    int decodedLen = base64_dec_len(encoded, encodedLen);
    char decoded[decodedLen];

    base64_decode(decoded, encoded, encodedLen);

    std::cout << decoded << std::endl;
    */

    if (argc != 3)
    {
        std::cerr << "Usage: Testclient <host> <port>\n";
        return 1;
    }

    for (int i = 1; i <= THREADS; i++)
    {
        try
        {
            std::thread t(func, argv);
            t.detach();
        }
        catch (std::exception& e)
        {
            std::cerr << "Exception: " << e.what() << "\n";
        }
    }

    int i = 0;
    while (1)
    {
        sleep(1);
        std::cout << ai_count - i << std::endl;
        i = ai_count;
    }
    return 0;
}
