from setuptools import setup

package_name = 'protonc'

setup(
    name=package_name,
    version="0.0.0",
    packages=[package_name],
    entry_points={
        "console_scripts": [
            "protonc_generator=protonc.protonc_generator:main",
        ],
    },
    python_requires=">=3.12",
)
