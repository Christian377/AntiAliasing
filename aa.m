system("COMMAND LINE TO EXECUTE")
oldDir = pwd;
mkdir('.build');
cd('.build');

[status, out] = system('cmake ..');

cd(oldDir);

if status ~= 0
    error('CMake failed:\n%s', out);
end
cd('bin')
[status, out] = system('aa');

% Clear workspace and command window
clc;
clear;
close all;

% List of filenames exactly as defined in your C code
files = { ...
    'aa_NONE.txt', ...
    'aa_MSAAx4.txt', 'aa_MSAAx8.txt', 'aa_MSAAx16.txt', ...
    'aa_FXAA.txt', 'aa_FXAA_Iterative.txt', ...
    'aa_SMAA_Low.txt', 'aa_SMAA_Medium.txt', 'aa_SMAA_High.txt', 'aa_SMAA_Ultra.txt' ...
};

% Labels for the X-axis
labels = { ...
    'No AA', ...
    'MSAA x4', 'MSAA x8', 'MSAA x16', ...
    'FXAA', 'FXAA (Iter)', ...
    'SMAA Low', 'SMAA Med', 'SMAA High', 'SMAA Ultra' ...
};

% Pre-allocate arrays for results
mean_times_ms = zeros(1, length(files));
files_found = false(1, length(files));

fprintf('Reading files...\n');

for i = 1:length(files)
    filename = files{i};
    
    if isfile(filename)
        try
           
            rawData = readmatrix(filename);
            
            % Remove any NaNs that result from trailing commas
            data = rmmissing(rawData);
            
            % Convert Nanoseconds to Milliseconds (1 ms = 1,000,000 ns)
            mean_val = mean(data, 'all');
            mean_times_ms(i) = mean_val / 1000000.0;
            
            files_found(i) = true;
            fprintf('Loaded %s: %.4f ms\n', filename, mean_times_ms(i));
        catch ME
            warning('Error reading %s: %s', filename, ME.message);
        end
    else
        warning('File not found: %s (Skipping)', filename);
    end
end

% Filter out missing files to avoid plotting empty zeros
plot_means = mean_times_ms(files_found);
plot_labels = labels(files_found);

if isempty(plot_means)
    error('No data files were found. Run your C application and save samples first.');
end

% Plotting Bar Chart
figure('Name', 'AA Performance Analysis', 'Color', 'w');

% Create the bar chart
b = bar(plot_means);

% Styling
b.FaceColor = 'flat'; % Allow individual coloring
b.EdgeColor = 'none';

% Color Logic: Assign colors based on algorithm family
% Adapts if some are missing
base_colors = [
    0.5 0.5 0.5; % No AA (Grey)
    0.2 0.6 0.8; % MSAA (Blue)
    0.2 0.6 0.8; 
    0.2 0.6 0.8;
    0.8 0.4 0.1; % FXAA (Orange)
    0.8 0.4 0.1;
    0.4 0.7 0.3; % SMAA (Green)
    0.4 0.7 0.3;
    0.4 0.7 0.3;
    0.4 0.7 0.3;
];

% Apply colors only to the bars that exist
current_indices = find(files_found);
for k = 1:length(current_indices)
    original_idx = current_indices(k);
    b.CData(k, :) = base_colors(original_idx, :);
end

% Axes configuration
ylabel('Frame Time (ms)');
title('Anti-Aliasing Performance Comparison');
grid on;
box off;

% Set X-axis labels
xticklabels(plot_labels);
xtickangle(45); % Tilt text for better readability

% 4. Add text values on top of bars
for i = 1:length(plot_means)
    text(i, plot_means(i), sprintf('%.2f', plot_means(i)), ...
        'HorizontalAlignment', 'center', ...
        'VerticalAlignment', 'bottom', ...
        'FontWeight', 'bold');
end

% Prevent labels from being cut off
ylim([0, max(plot_means) * 1.2]);