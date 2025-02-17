// Test runner for WebAssembly/WebGPU tests
async function runTests() {
    return new Promise((resolve, reject) => {
        try {
            // Call the test runner function exported from C++
            const result = Module.ccall('run_tests', 'number', [], []);
            
            // Parse the test results
            const success = result === 0;
            const details = Module.getTestResults(); // This will be implemented in C++
            
            resolve({
                success,
                details
            });
        } catch (error) {
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
