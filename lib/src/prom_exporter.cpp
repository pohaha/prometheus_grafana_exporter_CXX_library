#define INNER
#include <prom_exporter.h>

//LABEL
std::string Base_Label::to_string()
{
    std::string rt_string; 
    rt_string+=name + " = \"" + value + "\""; 
    return rt_string;
}

std::string Base_Label::to_string() const
{
    std::string rt_string; 
    rt_string+=name + " = \"" + value + "\""; 
    return rt_string;
}

Base_Label::Base_Label(const std::string& name,const std::string& value): name(name), value(value)
{}

bool Base_Label::operator<(const Base_Label& other)
{
    return (this->name)<other.name;
}
//END OF LABEL

//PUSHABLE

std::string Base_Pushable::to_string()
{
    std::string rt_string = "{";
    for(int i =0; i < labels.size(); i++)
    {
        rt_string+=labels[i].to_string()+ ", ";
    }
    rt_string.erase(rt_string.end()-2, rt_string.end());
    rt_string+= "} "+ std::to_string(data);
    return rt_string;
}
std::string Base_Pushable::to_string() const
{
    std::string rt_string = "{";
    for(int i =0; i < labels.size(); i++)
    {
        rt_string+=labels[i].to_string()+ ", ";
    }
    rt_string.erase(rt_string.end()-2, rt_string.end());
    rt_string+= "} "+ std::to_string(data);
    return rt_string;
}

void Base_Pushable::add_label(const Base_Label& label_to_add)
{
    labels.push_back(label_to_add);
}

std::string Base_Pushable::gen_map_key()
{
    std::vector<Base_Label> keyed_labels(labels);
    std::sort(keyed_labels.begin(), keyed_labels.end());
    std::string key;
    for(int i = 0; i < keyed_labels.size(); i++)
    {
        key+=keyed_labels[i].to_string();
    }
    return key;
}

std::string Base_Pushable::gen_map_key() const
{
    std::vector<Base_Label> keyed_labels(labels);
    std::sort(keyed_labels.begin(), keyed_labels.end());
    std::string key;
    for(int i = 0; i < keyed_labels.size(); i++)
    {
        key+=keyed_labels[i].to_string();
    }
    return key;
}

//END OF PUSHABLE

//METRIC

std::string Base_Metric::to_string()
{
    std::string rt_string = "";
    rt_string+= "# TYPE "+ metric_name + " "+ metric_type_as_string(metric_type) + "\n";
    rt_string+= "# HELP "+ metric_name + " "+ help_string + "\n";
    return rt_string;
}

std::string Base_Metric::to_string() const
{
    std::string rt_string = "";
    rt_string+= "# TYPE "+ metric_name + " "+ metric_type_as_string(metric_type) + "\n";
    rt_string+= "# HELP "+ metric_name + " "+ help_string + "\n";
    return rt_string;
}

void Base_Metric::add_pushable(const Base_Pushable& pushable)
{
    Prom_Exporter::pushed_data[metric_name][pushable.gen_map_key()] = pushable;
    pushables.push_back(pushable);
}

Base_Metric::Base_Metric(std::string name, Metric_Type type, std::string help_string): metric_type(type), help_string(help_string)
{
    metric_name = name;
    Prom_Exporter::metrics.push_back(*this);
}

std::string metric_type_as_string(Metric_Type type)
{
    switch (type)
    {
    case Metric_Type::gauge:
        return "gauge";
    case Metric_Type::counter:
        return "counter";
    case Metric_Type::histogram:
        return "histogram";
    default:
        return "none";
    }
}

//END OF METRIC

//EXPORTER

std::string Prom_Exporter::scrape()
        {
            std::string data_as_string;
            for(const auto metric : metrics)
            {
                std::string name = metric.metric_name;
                data_as_string = metric.to_string();
                for(const auto pushable : pushed_data[name])
                {
                    data_as_string+= name + pushable.second.to_string() + "\n";
                }
                data_as_string.erase(data_as_string.end()-1, data_as_string.end());
            }
            data_as_string+="\n";
            return data_as_string;
        }



void Prom_Exporter::proceed()
{

}
int Prom_Exporter::push_to_prometheus()
{
  CURL *curl;
  CURLcode res;
 
  /* In windows, this will init the winsock stuff */
  curl_global_init(CURL_GLOBAL_ALL);
 
  /* get a curl handle */
  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:9091/metrics/job/some_job");
    /* Now specify the POST data */
    std::string data =  Prom_Exporter::scrape();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, data.size()*sizeof(char));
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
 
    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 
    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
  return res;
}
//END OF EXPORTER
