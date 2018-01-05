%% File name: Histogram.m
%  Author: Ashutosh Modi
%  Description: Used to visualize the data by plotting histogram
%
%  Date: Nov 29, 2017.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%% Read CSV files
clc;clear all;
sourceDir = './';
dataFiles = dir(fullfile(sourceDir, '*.xlsx'));

data = {zeros()};
for i = 1:length(dataFiles)
    %Read xlsx files
    data{i} = xlsread(fullfile(sourceDir, dataFiles(i).name));
    % Find and remove those entries with duration as 0
    indices = find(data{i}(:,10)==0);
    data{i}(indices,:) = [];
    data{i} = sortrows(data{i},6);
end

%% Convert posix time to readable format
date = {zeros()};
realFirstDate = {zeros()};
realEndDate = {zeros()};
for i = 1:length(data)
    date(i) = {datetime(data{1,i}(:,1), 'convertfrom','posixtime')};
    realFirstDate(i) = {datetime((data{1,i}(:,6))/1000, 'convertfrom','epochtime','Epoch','1970-01-01')};
    realEndDate(i) = {datetime((data{1,i}(:,7))/1000, 'convertfrom','epochtime','Epoch','1970-01-01')};
    
    % Remove data entries for weekend
    % Check if first real date is weekend or not
    indicesWeekend = find(isweekend(realFirstDate{i}(:,1)) == 1);
    date{i}(indicesWeekend,:) = [];
    realFirstDate{i}(indicesWeekend,:) = [];
    realEndDate{i}(indicesWeekend,:) = [];
    data{i}(indicesWeekend,:) = [];
    
    indicesWeekend = [];
end


%% Plot Hour wise
[~, users] = size(data);
subPlotCount = ceil(sqrt(users));
for i=1:users
    subplot(subPlotCount,subPlotCount,i);histogram(datetime((data{1,i}(:,6))/1000, 'convertfrom','epochtime','Epoch','1970-01-01').Hour);
    title(i);
end