from setuptools import find_packages, setup

package_name = 'turtle_pkg'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='jiehuang',
    maintainer_email='hydrangeahuman@gmail.com',
    description='TODO: Package description',
    license='TODO: License declaration',
    extras_require={
        'test': [
            'pytest',
        ],
    },
    entry_points={
        'console_scripts': [
            'turtle_ctr_node = turtle_pkg.turtle_ctr_node:main',
            'turtle_gui_ctrl_node = turtle_pkg.turtle_gui_ctrl_node:main',
            'turtle_advice_ctrl_node = turtle_pkg.turtle_advice_ctrl_node:main'
        ],
    },
)
