#ifndef PROM_EXPORTER
    #define PROM_EXPORTER
    #include <map>
    #include <vector>
    #include <string>
    #include <algorithm>
    #include <curl/curl.h>
    #include <iostream>

    struct Base_Label
    {
        std::string name = "none";
        std::string value = "none";

        std::string to_string();

        std::string to_string() const;
        
        Base_Label(const std::string& name,const std::string& value);

        Base_Label() = default;


        bool operator<(const Base_Label& other);
    };

    class Base_Pushable
    {
    public:
        std::vector<Base_Label> labels;
        int data = 0;
        Base_Pushable() = default;
        std::string to_string();
        std::string to_string() const;

        void add_label(const Base_Label& label_to_add);

        std::string gen_map_key();

        std::string gen_map_key() const;

    };


    #define Init_label(name) \
    class name: public Base_Label\
    {\
    public:\
        name(std::string value): Base_Label(#name, value)\
        {}\
    };

    enum class Metric_Type
    {
        none,
        gauge,
        counter,
        histogram
    };

    class Base_Metric
    {
    public:
        std::string metric_name;
        std::vector<Base_Pushable> pushables;
        Metric_Type metric_type = Metric_Type::none;
        std::string help_string = "";

        Base_Metric(std::string name, Metric_Type type, std::string help_string);

        std::string to_string();

        std::string to_string() const;

        void add_pushable(const Base_Pushable& pushable);
    };

    std::string metric_type_as_string(Metric_Type type);

    class Prom_Exporter
    {
    public:
        static std::map<std::string, std::map<std::string, Base_Pushable>> pushed_data;
        static std::vector<Base_Metric> metrics;
        void proceed();
        static std::string scrape();

        static int push_to_prometheus();
    };
    
    #ifndef INNER
    //map definition!!
    std::map<std::string, std::map<std::string, Base_Pushable>> Prom_Exporter::pushed_data;

    //vector definition!!
    std::vector<Base_Metric> Prom_Exporter::metrics;
    #endif

#endif //PROM_EXPORTER