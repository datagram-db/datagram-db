library(sqldf)
library(ggplot2)
library(reshape2)
library(dplyr)
library(scales)
library(latex2exp)
require(scales)
require(ggpubr)
require(sqldf)
library(latex2exp)



t <- read.csv("commonsenseqatrain.txt_hists.csv",header = F)
names(t) <- c("Sample","d")
t1 <- sqldf("SELECT Sample, max(d) as m from t group by Sample")
names(t) <- c("Sample","values")
t$Sample <- as.character(t$Sample)
t1 <- t[t$Sample=="10",]
t2 <- t[t$Sample=="100",]
t3 <- t[t$Sample=="1000",]
t4 <- t[t$Sample=="10000",]
t5 <- t[t$Sample=="11230",]
p <- ggplot(NULL,aes(values)) +xlab("Trace Length") +ylab("Frequency")+  scale_x_log10(breaks = trans_breaks("log10", function(x) 10^x),
                                                                                       labels = trans_format("log10", math_format(10^.x))) 
scale_y_continuous(labels = scales::percent_format())
ggarrange( p+stat_bin(data=t1,fill="green",aes(y = after_stat(count / sum(count)))),
           p+ stat_bin(data=t2,fill="red",aes(y = after_stat(count / sum(count)))),
           p+stat_bin(data=t3,fill="blue",aes(y = after_stat(count / sum(count)))),
           p+stat_bin(data=t4,fill="orange",aes(y = after_stat(count / sum(count)))),
           p+stat_bin(data=t5,fill="pink",aes(y = after_stat(count / sum(count)))),
           labels=c("10¹","10²","10³","10⁴","11230"),
           ncol = 5, nrow = 1)


s <- read.csv("/home/giacomo/projects/gsm2/simple.csv")
s <- aggregate(cbind(loading_time,indexing_time,materialise_time_collection,materialise_time_final,query_collect_node_match,query_collect_edge_match,generate_nested_morphisms,run_transform)~data_name,data=s,FUN = mean)
s$load_index <- s$loading_time + s$indexing_time

t <- read.csv("/home/giacomo/projects/gsm2/scalability_dist.csv")
t$Loading <- t$loading_time
t$Indexing <- t$indexing_time
t$n_graphs <- t$n_graphs+1
t$Querying <- t$query_collect_node_match+t$query_collect_edge_match+t$generate_nested_morphisms+t$run_transform
t$Materialisation <- t$materialise_time_collection+t$materialise_time_final

t <- aggregate(cbind(Loading,Indexing,Querying,Materialisation)~ n_graphs,data=t,FUN = mean)
#t <- aggregate(cbind(loading_time,indexing_time,materialise_time_collection,materialise_time_final,query_collect_node_match,query_collect_edge_match,generate_nested_morphisms,run_transform)~ ngraphs+tokens,data=t,FUN = mean)
t <- melt(t, id=c("n_graphs"))
names(t) <- c( "n_graphs",  "Phase", "value" )

ggplot(t,aes(x=n_graphs,y=value,color=Phase,shape=Phase))+geom_line()+geom_point(size=4)+xlab(TeX("$|db|$"))+ylab("Time (ms)")+  scale_x_log10(breaks = trans_breaks("log10", function(x) 10^x),
                                                                                                                                               labels = trans_format("log10", math_format(10^.x))) +  scale_y_continuous(trans = log10_trans(),
                                                                                                                                                                                                                         breaks = trans_breaks("log10", function(x) 10^x),
                                                                                                                                                                                                                         labels = trans_format("log10", math_format(10^.x)))


t <- read.csv("/home/giacomo/projects/gsm2/scalability.csv")
t$Loading_Indexing <- t$loading_time + t$indexing_time
t$Querying <- t$query_collect_node_match+t$query_collect_edge_match+t$generate_nested_morphisms+t$run_transform
t$Materialisation <- t$materialise_time_collection+t$materialise_time_final

t <- aggregate(cbind(Loading_Indexing,Querying,Materialisation)~ ngraphs+tokens,data=t,FUN = mean)
#t <- aggregate(cbind(loading_time,indexing_time,materialise_time_collection,materialise_time_final,query_collect_node_match,query_collect_edge_match,generate_nested_morphisms,run_transform)~ ngraphs+tokens,data=t,FUN = mean)
t <- melt(t, id=c("tokens", "ngraphs"))
names(t) <- c("tokens" ,  "ngraphs",  "Phase", "value" )

ggplot(t,aes(x=ngraphs,y=value,color=Phase,shape=Phase))+geom_line()+geom_point(size=4)+facet_wrap(tokens ~ .,ncol=2)+xlab(TeX("$|db|$"))+ylab("Time (ms)")
ggplot(t,aes(x=tokens,y=value,color=Phase,shape=Phase))+geom_line()+geom_point(size=4)+facet_wrap(ngraphs ~ .,ncol=2)+xlab(TeX("$|O_i|$"))+ylab("Time (ms)")


+  scale_y_continuous(trans = log10_trans(),
                                                                                                                                  breaks = trans_breaks("log10", function(x) 10^x),
                                                                                                                                  labels = trans_format("log10", math_format(10^.x)))