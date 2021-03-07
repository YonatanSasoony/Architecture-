int digit_cnt(char* str){
    int counter = 0;
    if(str == 0)
        return counter;
    for(int i=0; ( (str[i] != 0) && (str[i] != 10) ) ; i++)
        if( (str[i] >= 48) && (str[i] <= 57) )
            counter++;
    return counter;
}

int main (int argc, char** argv){
    return 0;
}
