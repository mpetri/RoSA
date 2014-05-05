library(ggplot2)
library(scales)
library(gridExtra)
library(plyr)

pdf("matchlz_stats.pdf", onefile = TRUE,width=10,height=7)

# read data
data <- read.csv(file="../data/results/match_stats-2014-05-04-20-02-06.csv",sep=";")
data <- transform(data,method=ordered(method, levels = c("EXH","KMP","BMH","SA","MBMH")))
#data$method <- revalue(data$method, c("MBMH"="MBMH-CD-R", "SA"="SA-CD-R","BMH-CD"="BMH-CD-R"))


# decoded
decoded <- ggplot(data,aes(factor(plen),decoded,fill=method)) 
decoded <- decoded + facet_grid(. ~ R,labeller=label_both)
decoded <- decoded + geom_boxplot(outlier.size=1,outlier.shape=1)
decoded <- decoded + scale_y_log10(name="Decoded Symbols",breaks=c(10,100,1000,10000))
decoded <- decoded + annotation_logticks(sides = "lr")
decoded <- decoded + scale_x_discrete(name="Pattern Length")
decoded <- decoded + scale_fill_brewer(palette="Set1")
decoded <- decoded + ggtitle("Decoded symbols per COUNT qry for WEB-4GB")
grid.arrange(decoded)

# match_time
match_time <- ggplot(data,aes(factor(plen),match_time,fill=method)) 
match_time <- match_time + facet_grid(. ~ R,labeller=label_both)
match_time <- match_time + geom_boxplot(outlier.size=1,outlier.shape=1)
match_time <- match_time + scale_y_log10(name="Time [microseconds]",breaks=c(10,100,1000,10000))
match_time <- match_time + annotation_logticks(sides = "lr")
match_time <- match_time + scale_x_discrete(name="Pattern Length")
match_time <- match_time + scale_fill_brewer(palette="Set1")
match_time <- match_time + ggtitle("Time to perform matching per COUNT qry for WEB-4GB")
grid.arrange(match_time)

# percentage decoded
percent_decoded <- ggplot(data,aes(factor(plen),decoded/len_total,fill=method)) 
percent_decoded <- percent_decoded + geom_boxplot(outlier.size=1,outlier.shape=1)
percent_decoded <- percent_decoded + scale_y_continuous(name="Percentage of Symbols decoded",labels=percent)
percent_decoded <- percent_decoded + facet_grid(. ~ R,labeller=label_both)
percent_decoded <- percent_decoded + scale_x_discrete(name="Pattern Length")
percent_decoded <- percent_decoded + scale_fill_brewer(palette="Set1")
percent_decoded <- percent_decoded + ggtitle("Percentage of decoded symbols per COUNT qry for WEB-4GB")
grid.arrange(percent_decoded)

# time percent of count
# percent_time <- ggplot(data,aes(factor(plen),match_time/count_time,fill=method)) 
# percent_time <- percent_time + facet_grid(. ~ R,labeller=label_both)
# percent_time <- percent_time + geom_boxplot(outlier.size=1,outlier.shape=1)
# percent_time <- percent_time + scale_y_continuous(name="Percentage of COUNT time spent in match_lz",labels=percent)
# percent_time <- percent_time + scale_x_discrete(name="Pattern Length")
# percent_time <- percent_time + scale_fill_brewer(palette="Set1")
# percent_time <- percent_time + ggtitle("Percentage of time spent matching in factorized text per COUNT qry for WEB-4GB")
# grid.arrange(percent_time)

# # percentage of factors decoded
# percent_fdecoded <- ggplot(data,aes(factor(plen),ids_decoded/num_factors,fill=method)) 
# percent_fdecoded <- percent_fdecoded + facet_grid(. ~ R,labeller=label_both)
# percent_fdecoded <- percent_fdecoded + geom_boxplot()
# percent_fdecoded <- percent_fdecoded + scale_y_continuous(name="Percentage of at least partially factors decoded.",labels=percent)
# percent_fdecoded <- percent_fdecoded + scale_x_discrete(name="Pattern Length")
# percent_fdecoded <- percent_fdecoded + ggtitle("Percentage of at least partially factors decoded. per COUNT qry for WEB-4GB B=4096")
# grid.arrange(percent_fdecoded)



# # factor len data
# fdata <- subset(data,method=="BMH")
# fdata <- subset(fdata,R=="256")
# # average factor length per query
# avgfactorlen_hist <- ggplot(fdata,aes(avg_len))
# avgfactorlen_hist <- avgfactorlen_hist + facet_grid(B ~ .,labeller=label_both)
# avgfactorlen_hist <- avgfactorlen_hist + geom_histogram(binwidth=1)
# avgfactorlen_hist <- avgfactorlen_hist + scale_x_continuous(expand=c(0,0),name="Average Factor Length per Query",breaks=seq(0,200,10))
# avgfactorlen_hist <- avgfactorlen_hist + ggtitle("Histogram of Average Factor Length per Query WEB-4GB R=256")
# grid.arrange(avgfactorlen_hist)

# # # average factor length per query
# maxfactorlen_hist <- ggplot(fdata,aes(max_len)) 
# maxfactorlen_hist <- maxfactorlen_hist + facet_grid(B ~ .,labeller=label_both)
# maxfactorlen_hist <- maxfactorlen_hist + geom_histogram()
# maxfactorlen_hist <- maxfactorlen_hist + ggtitle("Histogram of Maximum Factor Length per Query WEB-4GB R=256")
# maxfactorlen_hist <- maxfactorlen_hist + scale_x_log10(expand=c(0,0),name="Maximum Factor Length per Query")
# maxfactorlen_hist <- maxfactorlen_hist + annotation_logticks(sides = "b")
# grid.arrange(maxfactorlen_hist)



dev.off();