library(ggplot2)
library(scales)
library(gridExtra)
library(plyr)

pdf("match_time.pdf", onefile = TRUE,width=10,height=7)

theme_complete_bw <- function(base_size = 12, base_family = "") {
  theme(
    line =               element_line(colour = "black", size = 0.5, linetype = 1,
                            lineend = "butt"),
    rect =               element_rect(fill = "white", colour = "black", size = 0.5, linetype = 1),
    text =               element_text(family = base_family, face = "plain",
                            colour = "black", size = base_size,
                            hjust = 0.5, vjust = 0.5, angle = 0, lineheight = 0.9),
    axis.text =          element_text(size = rel(0.8), colour = "grey50"),
    strip.text =         element_text(size = rel(0.8)),

    axis.line =          element_blank(),
    axis.text.x =        element_text(size = base_size * 0.8 , lineheight = 0.9, colour = "black", vjust = 1),
    axis.text.y =        element_text(size = base_size * 0.8, lineheight = 0.9, colour = "black", hjust = 1),
    axis.ticks =         element_line(colour = "black"),
    axis.title.x =       element_text(size = base_size * 1.2, vjust = 0.5),
    axis.title.y =       element_text(size = base_size * 1.2, angle = 90, vjust = 0.5),
    axis.ticks.length =  unit(0.15, "cm"),
    axis.ticks.margin =  unit(0.1, "cm"),

    legend.background =  element_blank(),
    legend.margin =      unit(0.2, "cm"),
    legend.key =         element_blank(),
    #legend.key.size =    unit(1.2, "lines"),
    legend.key.height =  unit(0.6, "cm"),
    legend.key.width =   unit(0.6, "cm"),
    legend.text =        element_text(size = rel(1)),
    legend.text.align =  NULL,
    legend.title =       element_blank(),
    legend.title.align = NULL,
    legend.direction =   NULL,
    legend.justification = "center",
    legend.box =         NULL,
    legend.position =    c(0.9,0.15),

    panel.background =   element_rect(fill = NA, colour = "grey", size = 1.3),
    panel.border =       element_blank(),
    panel.grid.major =   element_line(colour = "grey90", size = 0.7),
    panel.grid.minor =   element_line(colour = "grey90", size = 0.3),
    panel.margin =       unit(1.25, "lines"),

    strip.background =   element_rect(fill = NA, colour = NA),
    strip.text.x =       element_text(colour = "black", size = base_size),
    strip.text.y =       element_text(colour = "black", size = base_size, angle = -90),

    plot.background =    element_rect(colour = NA, fill = "white"),
    plot.title =         element_text(size = base_size * 1.2),
    plot.margin=         unit(c(5,0,5,0),"mm"),
    complete = TRUE
  )
}


# read data
data <- read.csv(file="../data/results/match_stats-2014-05-04-20-02-06.csv",sep=";")
data <- transform(data,method=ordered(method, levels = c("EXH","KMP","BMH","SA","MBMH")))
#data$method <- revalue(data$method, c("MBMH"="MBMH-CD-R", "SA"="SA-CD-R","BMH-CD"="BMH-CD-R"))

# decoded
plot <- ggplot(data,aes(factor(plen),match_time,fill=method)) 
plot <- plot + facet_grid(. ~ R,labeller=label_both)
plot <- plot + geom_boxplot(outlier.size=1,outlier.shape=1)
plot <- plot + scale_y_log10(name="Time [microseconds]",breaks=c(10,100,1000,10000))
plot <- plot + annotation_logticks(sides = "lr")
plot <- plot + scale_x_discrete(name="Pattern Length")
plot <- plot + theme_complete_bw()
plot <- plot + scale_fill_manual(values = c("#f4a582","#ca0020", "#bababa", "#ffffff", "#000000"))
print(plot)

dev.off()