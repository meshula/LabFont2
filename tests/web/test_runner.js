// Test runner for WebAssembly/WebGPU tests

// Initialize WebGPU before running tests
async function initializeWebGPU() {
    if (!navigator.gpu) {
        console.error("WebGPU is not supported in this browser.");
        throw new Error("WebGPU is not supported.");
    }

    try {
        const adapter = await navigator.gpu.requestAdapter({ powerPreference: 'high-performance' });
        if (!adapter) {
            throw new Error("Failed to get GPU adapter.");
        }

        const device = await adapter.requestDevice();
        if (!device) {
            throw new Error("Failed to get GPU device.");
        }

        console.log("WebGPU initialized successfully");

        // Store the device in the Module object so it's available for WebAssembly code
        Module.gpuDevice = device;
    } catch (error) {
        console.error("Error initializing WebGPU:", error);
        throw error;
    }
}


// Test runner for WebAssembly/WebGPU tests
async function runTests() {
    return new Promise(async (resolve, reject) => {
        try {
            // Ensure WebGPU is initialized
            if (!Module.gpuDevice) {
                console.error("WebGPU device not initialized");
                throw new Error("WebGPU device not initialized");
            }

            // Call the test runner function exported from C++
            console.log("Running tests...");
            const result = Module.ccall('run_tests', 'number', [], []);

            // Parse the test results
            const success = result === 0;
            const details = Module.getTestResults(); // This will be implemented in C++

            resolve({
                success,
                details
            });
        } catch (error) {
            console.error("Error running tests:", error);
            reject(error);
        }
    });
}

function displayResults(results) {
    const container = document.getElementById('results');
    
    results.forEach(suite => {
        const suiteDiv = document.createElement('div');
        suiteDiv.className = 'test-suite';
        
        const suiteTitle = document.createElement('h2');
        suiteTitle.textContent = suite.name;
        suiteDiv.appendChild(suiteTitle);
        
        suite.tests.forEach(test => {
            const testDiv = document.createElement('div');
            testDiv.className = 'test-case';
            
            const status = test.passed ? '✓' : '✗';
            const statusClass = test.passed ? 'pass' : 'fail';
            
            testDiv.innerHTML = `<span class="${statusClass}">${status}</span> ${test.name}`;
            
            if (!test.passed && test.message) {
                const errorDiv = document.createElement('div');
                errorDiv.className = 'error-message';
                errorDiv.style.marginLeft = '25px';
                errorDiv.style.color = '#e74c3c';
                errorDiv.textContent = test.message;
                testDiv.appendChild(errorDiv);
            }
            
            suiteDiv.appendChild(testDiv);
        });
        
        container.appendChild(suiteDiv);
    });
}

// Helper function to format test output
function formatTestOutput(output) {
    if (!output) return '';
    return output.split('\n').map(line => `  ${line}`).join('\n');
}

// WebGPU utility functions
async function getWebGPUAdapter() {
    if (!navigator.gpu) {
        throw new Error('WebGPU is not supported');
    }
    
    const adapter = await navigator.gpu.requestAdapter({
        powerPreference: 'high-performance'
    });
    
    if (!adapter) {
        throw new Error('No WebGPU adapter found');
    }
    
    return adapter;
}

async function getWebGPUDevice(adapter) {
    const device = await adapter.requestDevice({
        requiredFeatures: [],
        requiredLimits: {}
    });
    
    return device;
}

// Export WebGPU utilities to be used by the C++ code
Module.getWebGPUAdapter = getWebGPUAdapter;
Module.getWebGPUDevice = getWebGPUDevice;


// Entry point: Initialize WebGPU and then run tests
(async function () {
    try {
        await initializeWebGPU();
        const results = await runTests();
        displayResults(results.details);
    } catch (error) {
        console.error("Test execution failed:", error);
    }
})();

