#include<iostream>
#include<vector>
#include<thread>
#include<algorithm>
#include<numeric>

using namespace::std;

template<typename Iterator,typename T>
struct accumulate_block
{
    void operator()(Iterator first,Iterator last,T& result)
    {
        result=std::accumulate(first,last,result);
    }
};

template<typename Iterator,typename T>
T parallel_accumulate(Iterator first,Iterator last,T init)
{
    unsigned long const length=std::distance(first,last);
    if(!length)
        return init;
    unsigned long const min_per_thread=25;
    unsigned long const max_threads = (length+min_per_thread-1)/min_per_thread;//300+
    unsigned long const hardware_threads = std::thread::hardware_concurrency();//6
    unsigned long const num_threads = std::min(hardware_threads!=0?hardware_threads:2,max_threads);//6
    unsigned long const block_size=length/num_threads;//10000/6 = 1666

    std::vector<T> results(num_threads);//6
    std::vector<std::thread> threads(num_threads-1);//5
    Iterator block_start = first;
    for(unsigned long i=0;i<(num_threads-1);++i)
    {
        Iterator block_end=block_start;
        std::advance(block_end,block_size);
        threads[i]=std::thread(
            accumulate_block<Iterator,T>(),
            block_start,block_end,std::ref(results[i]));
        block_start=block_end;
    }//5
    for(int i = 0; i< num_threads-1; i++)
        std::cout << "Thread " << i+1 <<": " << threads[i].get_id() << std::endl;

    accumulate_block<Iterator,T>()(
    block_start,last,results[num_threads-1]);
    for(auto& entry: threads)
        entry.join();
    std::cout << "Thread " << num_threads << ": " << std::this_thread::get_id() << std::endl;
    return std::accumulate(results.begin(),results.end(),init);
}

int main(){
    vector<int> vec;
    for(int i = 0; i<10000;i++)
        vec.push_back(i);
    cout << parallel_accumulate(vec.cbegin(), vec.cend(),0) << endl;
    return 0;
}