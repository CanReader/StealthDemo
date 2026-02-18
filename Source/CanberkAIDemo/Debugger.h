#define message(...) if(GEngine) GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Red, __VA_ARGS__);
#define msg(x) if(GEngine) GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Red, TEXT(x));
#define msg_clr(x,color) if(GEngine) GEngine->AddOnScreenDebugMessage(-1,5.f,clr, x);
#define msg_t(x,t) if(GEngine) GEngine->AddOnScreenDebugMessage(-1,t,FColor::Red, x);


