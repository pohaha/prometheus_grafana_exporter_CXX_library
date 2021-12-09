#include<gtest/gtest.h>
#include <prom_exporter.h>

TEST(Gtest, Functions_properly) 
{
  EXPECT_EQ(1, 1);
}

std::string inet_help_string = "number of [transmitted,recieved] (direction) [bytes, packets,errors,drops] (monitored_data_type) on a specific [web interface] (device_name)";
class Inet_Metric: public Base_Metric
{
public:
    Init_label(device_name);
    Init_label(direction);
    Init_label(traced_entities);
    Inet_Metric(): Base_Metric("inet_metric", Metric_Type::counter, inet_help_string)
    {}
} metr_Inet;


TEST(basic_features, to_string)
{
    //Metric_Type should be converted from enum to string
    EXPECT_EQ(metric_type_as_string(Metric_Type::counter), "counter");

    //labels should be outputted in specified format
    std::vector<Base_Label> labels;
    labels.push_back(Inet_Metric::device_name("eno1"));
    labels.push_back(Inet_Metric::direction("tx"));
    EXPECT_EQ(labels[0].to_string(), "device_name = \"eno1\"");
    EXPECT_EQ(labels[1].to_string(), "direction = \"tx\"");

    //metrics should only output their own information, {TYPE, HELP}
    Base_Metric test_metric("test", Metric_Type::gauge, "help_string");
    std::string expected_output;
    expected_output += "# TYPE test gauge\n";
    expected_output += "# HELP test help_string\n";
    EXPECT_EQ(test_metric.to_string(), expected_output);

    //pushables should be responsible for showing label combination and current data
    Base_Pushable test_pushable{.labels = labels,
                                .data = 10
                               };
    EXPECT_EQ(test_pushable.to_string(), "{device_name = \"eno1\", direction = \"tx\"} 10");
    Prom_Exporter::metrics.erase(Prom_Exporter::metrics.begin()+1);
}

typedef std::vector<Base_Label> bl_t;
static bl_t const pgw0_rx = { Inet_Metric::device_name("pgw0"), Inet_Metric::direction("rx") };


TEST(basic_features, instantiation)
{
    //pushables should be instantiated using aggregate instantiation
    Base_Pushable some_pushable{
                                .labels =   pgw0_rx,
                                .data =     100
                                };
    EXPECT_EQ(some_pushable.to_string(), "{device_name = \"pgw0\", direction = \"rx\"} 100");

    //labels should be easily defined (using macros?)
    Init_label(traced_entities);
    EXPECT_EQ(traced_entities("errors").to_string(), "traced_entities = \"errors\"");
}

TEST(logic, pushable_aggregation)
{
    //all pushables added to any metric should be aggregated in exporter
    Base_Pushable some_pushable{.labels = {
                                    Inet_Metric::device_name("eno1"),
                                    Inet_Metric::direction("tx")
                                    },
                                .data = 100};
    std::string uid = some_pushable.gen_map_key();
    metr_Inet.add_pushable(some_pushable);
    EXPECT_EQ(some_pushable.to_string(), Prom_Exporter::pushed_data["inet_metric"][uid].to_string());

    //any pushable with the same label names and values should be replaced by the last entry, independent of label order
    //uids should be equal, and value in exporter should be as in other_pushable - as it is the last one
    Base_Pushable other_pushable{.labels = {Inet_Metric::direction("tx"),
                                            Inet_Metric::device_name("eno1")},
                                 .data = 200};

    std::string uid_two = other_pushable.gen_map_key();
    metr_Inet.add_pushable(other_pushable);
    EXPECT_EQ(uid, uid_two);
    EXPECT_EQ(Prom_Exporter::pushed_data["inet_metric"][uid].data, 200);
    EXPECT_EQ(Prom_Exporter::pushed_data["inet_metric"][uid_two].data, 200);


    Prom_Exporter::pushed_data.clear(); //used for further clear testing
    metr_Inet.pushables.clear(); //used for further clear testing
}

TEST(logic, final_data_output)
{
    //final data should be readable by pushgateway
    Base_Pushable some_pushable{.labels = {
                                            Inet_Metric::device_name("test_device_name"),
                                            Inet_Metric::direction("test_dir")},
                                .data = 1234};
    metr_Inet.add_pushable(some_pushable);
    EXPECT_EQ(metr_Inet.to_string()+metr_Inet.metric_name+some_pushable.to_string()+"\n", Prom_Exporter::scrape());
    Prom_Exporter::pushed_data.clear(); //used for further clear testing
    metr_Inet.pushables.clear(); //used for further clear testing
}

TEST(final_tests, server_output)
{
    Init_label(server);
    //aggregated data should be pushed to a pushgateway server, whose port is specked by pushgateway configuration
    Base_Pushable pushed_to_pushgateway{.labels  = {server("prom_pgw")},
                                        .data = 400};
    metr_Inet.add_pushable(pushed_to_pushgateway);
    EXPECT_EQ(CURLE_OK, Prom_Exporter::push_to_prometheus());
}