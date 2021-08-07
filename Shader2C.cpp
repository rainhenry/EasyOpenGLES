/**********************************************************************

    程序名称:将C格式的文本shader转换为C语法的纯数组
    程序版本:REV 0.1
    设计编写:rainhenry
    创建日期:20201213

    版本修订：
        REV 0.1  20201211  rainhenry   创建文档

    设计说明
        用于转换格式调试shader排查问题定位方便,编写方便,可以同时生成
    多个shader到同一个文件中.

**********************************************************************/
//---------------------------------------------------------------------
//  包含头文件
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include "IncludeOpenGLES.h"

//---------------------------------------------------------------------
//  相关宏定义

//  读取一次的缓冲区大小
#define READ_BUFFER_SIZE              4096
#define WRITE_SWAP_CNT                32    //  每写入一些字节数后换行
#define DEBUG_LOG                     0     //  是否开启打印Log

//---------------------------------------------------------------------
//  相关类型定义

//  输入类型定义
typedef enum
{
    EInputType_None = 0,       //  正常输入,可以为文件名,也可以为开关选项
    EInputType_OutputName,     //  当为输出文件名
}EInputType;

//---------------------------------------------------------------------
//  相关变量

//  当前输入类型
EInputType CurrentInputType = EInputType_None;   //  默认为正常输入

//  输出文件相关
std::string OutputFileName = "ShaderSrc.c";      //  输出的文件名
std::vector<std::string> InputFileVec;           //  输入的文件容器 

//  读取文件的缓冲区
unsigned char ReadBuffer[READ_BUFFER_SIZE];

//---------------------------------------------------------------------
//  将字符串中全部的不可识别字符都替换成下划线
std::string ReplaceStringOther2Underline(std::string in_str)
{
    int len = in_str.size();
    int i=0;
    for(i=0;i<len;i++)
    {
        if(in_str.at(i) == '.')
        {
            in_str.replace(i, 1, 1, '_');
        }
        else if(in_str.at(i) == '/')
        {
            in_str.replace(i, 1, 1, '_');
        }
        else if(in_str.at(i) == '\\')
        {
            in_str.replace(i, 1, 1, '_');
        }
        else if(in_str.at(i) == ' ')
        {
            in_str.replace(i, 1, 1, '_');
        }
        else if(in_str.at(i) == ':')
        {
            in_str.replace(i, 1, 1, '_');
        }
        else if(in_str.at(i) == ',')
        {
            in_str.replace(i, 1, 1, '_');
        }
        else if(in_str.at(i) == '(')
        {
            in_str.replace(i, 1, 1, '_');
        }
        else if(in_str.at(i) == '}')
        {
            in_str.replace(i, 1, 1, '_');
        }
        else if(in_str.at(i) == '[')
        {
            in_str.replace(i, 1, 1, '_');
        }
        else if(in_str.at(i) == ']')
        {
            in_str.replace(i, 1, 1, '_');
        }
        else if(in_str.at(i) == '{')
        {
            in_str.replace(i, 1, 1, '_');
        }
        else if(in_str.at(i) == '}')
        {
            in_str.replace(i, 1, 1, '_');
        }
        else if(in_str.at(i) == '-')
        {
            in_str.replace(i, 1, 1, '_');
        }
        else if(in_str.at(i) == '+')
        {
            in_str.replace(i, 1, 1, '_');
        }
        else if(in_str.at(i) == '!')
        {
            in_str.replace(i, 1, 1, '_');
        }
    }
    return in_str;
}

//---------------------------------------------------------------------
//  将文件名的.c改为.h
std::string FileNameC2H(std::string in_name)
{
    int len = in_name.size();
    return in_name.replace(len-1, 1, 1, 'h');
};

//---------------------------------------------------------------------
//  将指定的文件的内容转换为数组写入到文件中
//  成功返回0, 失败返回其他值
int FileToArray(FILE* pfile_c, FILE* pfile_h, std::string in_file_name)
{
    //  打开文件
    FILE* pinfile = fopen(in_file_name.c_str(), "rb");

    //  检查文件是否打开成功
    if(pinfile == 0)
    {
        printf("Open File Error!! File Name:%s\r\n", in_file_name.c_str());
        return -1;
    }

    //  生成文件信息头部
    fprintf(pfile_c, "const char %s[] = \r\n{\r\n", ReplaceStringOther2Underline(in_file_name).c_str());
    fprintf(pfile_h, "extern const  char %s[];\r\n", ReplaceStringOther2Underline(in_file_name).c_str());

    //  字节计数器
    int byte_cnt = 0;

    //  循环读取全部文件
    do
    {
        //  读取1个缓冲区大小
        int read_len = fread(ReadBuffer, 1, READ_BUFFER_SIZE, pinfile);

        //  根据实际读取数目,向文件写入数据
        int i=0;
        for(i=0;i<read_len;i++)
        {
            fprintf(pfile_c, "0x%02X,", ReadBuffer[i] & 0x0FF);
            byte_cnt++;

            if(byte_cnt >= WRITE_SWAP_CNT)
            {
                byte_cnt = 0;
                fprintf(pfile_c, "\r\n");
            }
        }
    }
    //  当文件到末尾
    while(!feof(pinfile));

    //  当读取完成时,不在末尾换行
    if(byte_cnt < WRITE_SWAP_CNT)
    {
        fprintf(pfile_c, "\r\n");
    }

    //  生成文件结束信息
    fprintf(pfile_c, "};\r\n\r\n");

    //  关闭文件
    fclose(pinfile);

    //  操作成功
    return 0;
}

//---------------------------------------------------------------------
//  主函数
int main(int argc, char** argv)
{
    //  打印提示信息
#if DEBUG_LOG
    printf("-------------Shader To C--------------\r\n");
    printf("-----------20201211 REV 0.1-----------\r\n");
#endif  //  DEBUG_LOG

    //  检查输入参数
    if(argc <= 1)
    {
        printf("Input Arg Number Error!!\r\n");
        return -1;
    }

    //  遍历全部输入参数
    int i=0;
    for(i=1;i<argc;i++)
    {
        //  当为正常输入类型
        if(CurrentInputType == EInputType_None)
        {
            //  当为表示输出文件名设置的开关
            if(strcmp("-o", argv[i]) == 0)
            {
                CurrentInputType = EInputType_OutputName;
            }
            //  其他情况
            else
            {
                //  将文件名插入输入文件列表中
                InputFileVec.insert(InputFileVec.end(), argv[i]);
            }
        }
        //  当为输出文件名
        else if(CurrentInputType == EInputType_OutputName)
        {
            //  设置输出文件名
            OutputFileName = argv[i];

            //  恢复开关到默认
            CurrentInputType = EInputType_None;
        }
        //  错误类型
        else
        {
            printf("Error Input Type!!\r\n");
            return -2;
        }
    }

    //  打印识别结果
#if DEBUG_LOG
    printf("Output File Name:%s\r\n", OutputFileName.c_str());
#endif  //  DEBUG_LOG
    int input_file_total = InputFileVec.size();
#if DEBUG_LOG
    printf("Total Input File Count is %d\r\n", input_file_total);
    printf("Input File List:\r\n");
    for(i=0;i<input_file_total;i++)
    {
        printf("    %s\r\n", InputFileVec.at(i).c_str());
    }
#endif  //  DEBUG_LOG

    //  创建写入文件
    FILE* poutfile_c = fopen(OutputFileName.c_str(), "wb");
    FILE* poutfile_h = fopen(FileNameC2H(OutputFileName).c_str(), "wb");

    //  检查文件是否创建成功
    if((poutfile_c == 0) || (poutfile_h == 0))
    {
        printf("Output File Create Error!!\r\n");
        return -3;
    }

    //  生成文件头部信息
    fprintf(poutfile_h, "#ifndef __SHADERSRC_H__\r\n");
    fprintf(poutfile_h, "#define __SHADERSRC_H__\r\n");
    fprintf(poutfile_h, "#ifdef __cplusplus\r\n");
    fprintf(poutfile_h, "extern \"C\"\r\n");
    fprintf(poutfile_h, "{\r\n");
    fprintf(poutfile_h, "#endif  //  __cplusplus\r\n");

    //  遍历所有文件
    for(i=0;i<input_file_total;i++)
    {
        FileToArray(poutfile_c, poutfile_h, InputFileVec.at(i));
    }

    //  生成文件尾部信息
    fprintf(poutfile_h, "#ifdef __cplusplus\r\n");
    fprintf(poutfile_h, "}\r\n");
    fprintf(poutfile_h, "#endif  //  __cplusplus\r\n");
    fprintf(poutfile_h, "#endif  //  __SHADERSRC_H__\r\n");

    //  关闭文件释放资源
    fclose(poutfile_c);
    fclose(poutfile_h);

    //  程序正常结束
    return 0;
}


